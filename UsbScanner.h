#pragma once

// ============================================================================
//  UsbScanner.h  —  USB 扫描核心类（纯 C++17，脱离 UI 框架）
//  ============================================================================
//  职责：
//    1. 枚举系统中所有可移动存储设备 (DRIVE_REMOVABLE)
//    2. 递归扫描指定驱动器，按后缀名分类所有文件
//    3. 提供可节流的进度回调 & 原子中断标志
//
//  依赖：仅 Windows API + C++17 std::filesystem，无 Qt / MFC 依赖
// ============================================================================

#include <cstdint>
#include <atomic>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

// ---------------------------------------------------------------------------
//  枚举：文件大类
// ---------------------------------------------------------------------------
enum class FileCategory {
    Video,        // 视频
    Audio,        // 音频
    Image,        // 图片
    Document,     // 文档
    Archive,      // 压缩包
    Executable,   // 可执行文件
    Other         // 其他
};

// ---------------------------------------------------------------------------
//  数据结构：驱动器信息
// ---------------------------------------------------------------------------
struct DriveInfo {
    std::wstring driveLetter;   // 盘符，如 L"D:"
    std::wstring volumeLabel;   // 卷标，如 L"KINGSTON"
    std::wstring fileSystem;    // 文件系统，如 L"FAT32" / L"exFAT" / L"NTFS"
    uint64_t     totalBytes;    // 总容量（字节）
    uint64_t     freeBytes;     // 可用容量（字节）
    bool         isRemovable;   // 是否为可移动设备
};

// ---------------------------------------------------------------------------
//  数据结构：文件条目
// ---------------------------------------------------------------------------
struct FileItem {
    std::wstring fullPath;      // 完整路径，如 L"D:\\Movies\\demo.mp4"
    std::wstring fileName;      // 文件名（含后缀），如 L"demo.mp4"
    std::wstring extension;     // 后缀名（小写，无点），如 L"mp4"；无后缀则为空串
    uint64_t     fileSize;      // 文件大小（字节）
    FileCategory category;      // 所属大类
};

// ---------------------------------------------------------------------------
//  进度回调类型
//  参数：fileCount - 当前已扫描到的文件总数
//        currentPath - 当前正在处理的文件完整路径
// ---------------------------------------------------------------------------
using ProgressCallback = std::function<void(int fileCount, const std::wstring& currentPath)>;

// ===========================================================================
//  UsbScanner  —  底层扫描核心类
// ===========================================================================
class UsbScanner {
public:
    UsbScanner();
    ~UsbScanner();

    // 禁止拷贝
    UsbScanner(const UsbScanner&) = delete;
    UsbScanner& operator=(const UsbScanner&) = delete;

    // -----------------------------------------------------------------------
    //  设置进度回调
    //  - callback          : 回调函数（在扫描线程中被调用）
    //  - throttleInterval  : 节流间隔，每扫描 N 个文件才触发一次回调，默认 1000
    // -----------------------------------------------------------------------
    void setProgressCallback(ProgressCallback callback, int throttleInterval = 1000);

    // -----------------------------------------------------------------------
    //  枚举当前系统中所有可移动驱动器（U 盘 / 移动硬盘）
    //  返回 DriveInfo 列表 — 仅 DRIVE_REMOVABLE
    //  内部调用 GetLogicalDrives / GetDriveTypeW / GetVolumeInformationW
    // -----------------------------------------------------------------------
    std::vector<DriveInfo> enumerateRemovableDrives();

    // -----------------------------------------------------------------------
    //  枚举系统中所有就绪的驱动器（U 盘 + 本地硬盘），排除光驱和网络盘
    //  返回 DriveInfo 列表 — DRIVE_REMOVABLE + DRIVE_FIXED
    //  供 UI 团队做"所有盘扫描"时使用
    // -----------------------------------------------------------------------
    std::vector<DriveInfo> enumerateDrives();

    // -----------------------------------------------------------------------
    //  扫描指定驱动器根目录（如 L"D:\\"），返回所有文件条目
    //  内部使用 std::filesystem 递归遍历，自动跳过无权限目录
    //  可在任意时刻调用 stop() 中断扫描
    // -----------------------------------------------------------------------
    std::vector<FileItem> scanDrive(const std::wstring& driveRoot);

    // -----------------------------------------------------------------------
    //  请求终止扫描（线程安全，可跨线程调用）
    // -----------------------------------------------------------------------
    void stop();

    // -----------------------------------------------------------------------
    //  查询是否正在扫描
    // -----------------------------------------------------------------------
    bool isScanning() const;

    // -----------------------------------------------------------------------
    //  根据小写后缀名（无点）分类文件
    //  静态方法，可供外部独立使用
    // -----------------------------------------------------------------------
    static FileCategory classifyFile(const std::wstring& lowerExtension);

private:
    // ---- 内部状态 ----------------------------------------------------------
    std::atomic<bool> m_cancelFlag;
    std::atomic<bool> m_isScanning;
    int               m_throttleInterval = 1000;
    ProgressCallback  m_progressCallback;

    // ---- 后缀名 → 大类映射表 ------------------------------------------------
    static const std::unordered_map<std::wstring, FileCategory>& getCategoryMap();
};
