// ============================================================================
//  UsbScanner.cpp  —  USB 扫描核心类实现
// ============================================================================

#include "UsbScanner.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <cwctype>
#include <filesystem>

namespace fs = std::filesystem;

// ===========================================================================
//  构造 / 析构
// ===========================================================================

UsbScanner::UsbScanner()
    : m_cancelFlag(false)
    , m_isScanning(false)
    , m_throttleInterval(1000)
{}

UsbScanner::~UsbScanner() {
    stop();   // 确保析构时通知扫描线程退出
}

// ===========================================================================
//  公开接口
// ===========================================================================

void UsbScanner::setProgressCallback(ProgressCallback callback, int throttleInterval) {
    m_progressCallback = std::move(callback);
    m_throttleInterval = (throttleInterval > 0) ? throttleInterval : 1;
}

void UsbScanner::stop() {
    m_cancelFlag.store(true, std::memory_order_release);
}

bool UsbScanner::isScanning() const {
    return m_isScanning.load(std::memory_order_acquire);
}

// ===========================================================================
//  后缀名 → 大类 映射表（惰性初始化，线程安全）
// ===========================================================================

const std::unordered_map<std::wstring, FileCategory>& UsbScanner::getCategoryMap() {
    static const std::unordered_map<std::wstring, FileCategory> s_map = [] {
        std::unordered_map<std::wstring, FileCategory> m;

        // -- 视频 -----------------------------------------------------------
        for (const auto* ext : { L"mp4", L"avi", L"mkv", L"mov", L"wmv",
                                 L"flv", L"webm", L"m4v", L"mpg", L"mpeg",
                                 L"3gp", L"3g2", L"rmvb", L"rm", L"ts",
                                 L"m2ts", L"mts", L"vob", L"ogv", L"asf",
                                 L"divx", L"xvid", L"f4v", L"swf" }) {
            m[ext] = FileCategory::Video;
        }

        // -- 音频 -----------------------------------------------------------
        for (const auto* ext : { L"mp3", L"wav", L"flac", L"aac", L"ogg",
                                 L"wma", L"m4a", L"ape", L"alac", L"opus",
                                 L"aiff", L"aif", L"au", L"mid", L"midi",
                                 L"ra",  L"amr", L"ac3", L"dts", L"wv",
                                 L"cda", L"mp2", L"pcm" }) {
            m[ext] = FileCategory::Audio;
        }

        // -- 图片 -----------------------------------------------------------
        for (const auto* ext : { L"jpg", L"jpeg", L"png", L"gif", L"bmp",
                                 L"tiff", L"tif", L"webp", L"svg", L"ico",
                                 L"psd", L"raw", L"cr2", L"nef", L"arw",
                                 L"dng", L"heic", L"heif", L"jp2", L"jxr",
                                 L"eps", L"ai",  L"cdr" }) {
            m[ext] = FileCategory::Image;
        }

        // -- 文档 -----------------------------------------------------------
        for (const auto* ext : { L"pdf", L"doc", L"docx", L"xls", L"xlsx",
                                 L"ppt", L"pptx", L"txt", L"csv", L"md",
                                 L"json", L"xml", L"html", L"htm", L"rtf",
                                 L"odt", L"ods", L"odp", L"pages", L"numbers",
                                 L"key", L"tex", L"log", L"ini", L"cfg",
                                 L"yaml", L"yml", L"toml" }) {
            m[ext] = FileCategory::Document;
        }

        // -- 压缩包 ---------------------------------------------------------
        for (const auto* ext : { L"zip", L"rar", L"7z", L"tar", L"gz",
                                 L"bz2", L"xz", L"iso", L"cab", L"arj",
                                 L"lzh", L"lha", L"zst", L"tgz", L"tbz2",
                                 L"txz", L"tlz", L"ace" }) {
            m[ext] = FileCategory::Archive;
        }

        // -- 可执行文件 -----------------------------------------------------
        for (const auto* ext : { L"exe", L"dll", L"msi", L"bat", L"cmd",
                                 L"com", L"scr", L"pif", L"sys", L"cpl",
                                 L"ps1", L"vbs", L"wsf" }) {
            m[ext] = FileCategory::Executable;
        }

        return m;
    }();

    return s_map;
}

FileCategory UsbScanner::classifyFile(const std::wstring& lowerExtension) {
    if (lowerExtension.empty()) {
        return FileCategory::Other;
    }
    const auto& map = getCategoryMap();
    auto it = map.find(lowerExtension);
    return (it != map.end()) ? it->second : FileCategory::Other;
}

// ===========================================================================
//  驱动器枚举（Windows API）
// ===========================================================================

std::vector<DriveInfo> UsbScanner::enumerateRemovableDrives() {
    std::vector<DriveInfo> drives;

    const DWORD drivesMask = GetLogicalDrives();
    if (drivesMask == 0) {
        return drives;   // 调用失败，返回空列表
    }

    for (int i = 0; i < 26; ++i) {
        if (!(drivesMask & (1 << i))) {
            continue;    // 该盘符不存在
        }

        // 构造根路径，如 L"E:\\"
        wchar_t rootPath[4] = {
            static_cast<wchar_t>(L'A' + i),
            L':',
            L'\\',
            L'\0'
        };

        const UINT driveType = GetDriveTypeW(rootPath);

        // 仅收集可移动设备；也收集 DRIVE_FIXED 中可能是 USB 移动硬盘的
        // 用户如需扫描固定硬盘，可自行扩展此判断
        if (driveType != DRIVE_REMOVABLE) {
            continue;
        }

        DriveInfo info;
        info.driveLetter  = std::wstring(1, static_cast<wchar_t>(L'A' + i)) + L":";
        info.isRemovable  = true;

        // -- 获取卷标 & 文件系统 --------------------------------------------
        wchar_t volName[MAX_PATH + 1] = { 0 };
        wchar_t fsName[MAX_PATH + 1]  = { 0 };
        DWORD   serialNumber = 0;
        DWORD   maxCompLen   = 0;
        DWORD   fsFlags      = 0;

        if (GetVolumeInformationW(rootPath,
                                  volName, MAX_PATH + 1,
                                  &serialNumber, &maxCompLen,
                                  &fsFlags,
                                  fsName, MAX_PATH + 1)) {
            info.volumeLabel = volName;
            info.fileSystem  = fsName;
        } else {
            // 驱动器可能未就绪（无媒体），跳过
            continue;
        }

        // -- 获取磁盘空间 --------------------------------------------------
        ULARGE_INTEGER freeBytesAvailable{};
        ULARGE_INTEGER totalNumberOfBytes{};
        ULARGE_INTEGER totalNumberOfFreeBytes{};

        if (GetDiskFreeSpaceExW(rootPath,
                                &freeBytesAvailable,
                                &totalNumberOfBytes,
                                &totalNumberOfFreeBytes)) {
            info.totalBytes = totalNumberOfBytes.QuadPart;
            info.freeBytes  = freeBytesAvailable.QuadPart;
        } else {
            info.totalBytes = 0;
            info.freeBytes  = 0;
        }

        drives.push_back(std::move(info));
    }

    return drives;
}

// ===========================================================================
//  驱动器枚举（所有就绪盘 — 可移动 + 本地固定）
// ===========================================================================

std::vector<DriveInfo> UsbScanner::enumerateDrives() {
    std::vector<DriveInfo> drives;

    const DWORD drivesMask = GetLogicalDrives();
    if (drivesMask == 0) {
        return drives;
    }

    for (int i = 0; i < 26; ++i) {
        if (!(drivesMask & (1 << i))) {
            continue;
        }

        wchar_t rootPath[4] = {
            static_cast<wchar_t>(L'A' + i),
            L':',
            L'\\',
            L'\0'
        };

        const UINT driveType = GetDriveTypeW(rootPath);

        // 只收集可移动设备 和 本地固定盘；排除光驱、网络盘、RAM 盘
        if (driveType != DRIVE_REMOVABLE && driveType != DRIVE_FIXED) {
            continue;
        }

        DriveInfo info;
        info.driveLetter  = std::wstring(1, static_cast<wchar_t>(L'A' + i)) + L":";
        info.isRemovable  = (driveType == DRIVE_REMOVABLE);

        // -- 获取卷标 & 文件系统 --------------------------------------------
        wchar_t volName[MAX_PATH + 1] = { 0 };
        wchar_t fsName[MAX_PATH + 1]  = { 0 };
        DWORD   serialNumber = 0;
        DWORD   maxCompLen   = 0;
        DWORD   fsFlags      = 0;

        if (GetVolumeInformationW(rootPath,
                                  volName, MAX_PATH + 1,
                                  &serialNumber, &maxCompLen,
                                  &fsFlags,
                                  fsName, MAX_PATH + 1)) {
            info.volumeLabel = volName;
            info.fileSystem  = fsName;
        } else {
            // 驱动器未就绪（如空光驱），跳过
            continue;
        }

        // -- 获取磁盘空间 --------------------------------------------------
        ULARGE_INTEGER freeBytesAvailable{};
        ULARGE_INTEGER totalNumberOfBytes{};
        ULARGE_INTEGER totalNumberOfFreeBytes{};

        if (GetDiskFreeSpaceExW(rootPath,
                                &freeBytesAvailable,
                                &totalNumberOfBytes,
                                &totalNumberOfFreeBytes)) {
            info.totalBytes = totalNumberOfBytes.QuadPart;
            info.freeBytes  = freeBytesAvailable.QuadPart;
        } else {
            info.totalBytes = 0;
            info.freeBytes  = 0;
        }

        drives.push_back(std::move(info));
    }

    return drives;
}

// ===========================================================================
//  驱动器扫描（std::filesystem 递归遍历）
// ===========================================================================

std::vector<FileItem> UsbScanner::scanDrive(const std::wstring& driveRoot) {
    std::vector<FileItem> results;

    // ---- 重置状态 ---------------------------------------------------------
    m_cancelFlag.store(false, std::memory_order_release);
    m_isScanning.store(true, std::memory_order_release);

    int fileCount = 0;

    // ---- 预分配空间（避免反复 realloc）-------------------------------------
    results.reserve(50000);

    try {
        // 使用 recursive_directory_iterator，启用 skip_permission_denied
        // 不加 follow_directory_symlink，避免符号链接死循环
        fs::recursive_directory_iterator it(
            fs::path(driveRoot),
            fs::directory_options::skip_permission_denied
        );
        fs::recursive_directory_iterator end;

        while (it != end) {

            // ---- 中断检查 -------------------------------------------------
            if (m_cancelFlag.load(std::memory_order_acquire)) {
                break;
            }

            // ---- 安全处理每个条目 -----------------------------------------
            try {
                const fs::directory_entry& entry = *it;

                // 仅处理普通文件；跳过目录、符号链接等
                if (!entry.is_regular_file()) {
                    ++it;
                    continue;
                }

                // 构造 FileItem
                FileItem item;
                const fs::path& p = entry.path();

                item.fullPath = p.wstring();
                item.fileName = p.filename().wstring();

                // -- 后缀名处理（去点、转小写） --------------------------
                std::wstring rawExt = p.extension().wstring();
                if (!rawExt.empty() && rawExt[0] == L'.') {
                    rawExt.erase(0, 1);
                }
                std::wstring lowerExt;
                lowerExt.reserve(rawExt.size());
                for (wchar_t c : rawExt) {
                    lowerExt.push_back(static_cast<wchar_t>(std::towlower(c)));
                }
                item.extension = lowerExt;

                // -- 文件大小 -------------------------------------------
                try {
                    item.fileSize = entry.file_size();
                } catch (const fs::filesystem_error&) {
                    item.fileSize = 0;
                }

                // -- 分类 -----------------------------------------------
                item.category = classifyFile(lowerExt);

                results.push_back(std::move(item));
                ++fileCount;

                // -- 节流进度回调 ---------------------------------------
                if (m_progressCallback && (fileCount % m_throttleInterval == 0)) {
                    // 回调中传当前文件的路径
                    m_progressCallback(fileCount, results.back().fullPath);
                }

                ++it;

            } catch (const fs::filesystem_error&) {
                // 单个条目处理失败（文件被删除 / 权限变更），跳过并继续
                // 先禁用当前目录的递归（防止无限重试），再前进
                if (it != end) {
                    it.disable_recursion_pending();
                    try {
                        ++it;
                    } catch (...) {
                        break;   // 迭代器彻底损坏，退出扫描
                    }
                }
            }
        }   // end while

    } catch (const fs::filesystem_error&) {
        // 根目录本身无法访问；results 中保留已收集到的任何文件
        // 这是预期的静默处理，无需上报
    }

    // ---- 当前尚未触发的剩余文件，补发一次回调告知最终进度 ------------------
    if (m_progressCallback && (fileCount % m_throttleInterval != 0)) {
        m_progressCallback(fileCount, L"");
    }

    m_isScanning.store(false, std::memory_order_release);
    return results;
}
