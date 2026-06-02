#include "scanner.h"
#include <winioctl.h>

#include <Windows.h>
#include <algorithm>
#include <chrono>
#include <cwctype>
#include <filesystem>
#include <functional>
#include <unordered_map>

#define BUFFER_SIZE (1024 * 64)

scanner::Scan_Result scanner::scan_drive(char drive_letter) {
    Scan_Result result;

    //0. 判断驱动类型：可移动设备跳过 MFT
    bool try_mft = false;
    {
        wchar_t rootPath[4] = {
            static_cast<wchar_t>(drive_letter), L':', L'\\', L'\0'
        };
        try_mft = (GetDriveTypeW(rootPath) != DRIVE_REMOVABLE);
    }

    //1. NTFS 硬盘 → MFT；FAT32/exFAT 可移动盘 → 直接走 std::filesystem
    if (try_mft) {
        std::string vol_path = "\\\\.\\";
        vol_path += drive_letter;
        vol_path += ":";

        HANDLE hVol = CreateFileA(
        vol_path.c_str(),
        GENERIC_READ,                           //只读
        FILE_SHARE_READ | FILE_SHARE_WRITE,     //不影响其他文件
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hVol != INVALID_HANDLE_VALUE) {
        //2.初始化MFT参数
        MFT_ENUM_DATA mftEnumData;
        mftEnumData.StartFileReferenceNumber = 0;
        mftEnumData.LowUsn = 0;
        mftEnumData.HighUsn = MAXLONGLONG;

        BYTE buffer[BUFFER_SIZE];
        DWORD byteCount = 0;

        result.nodes.reserve(2000000);
        result.id_to_index.reserve(2000000);
        result.parent_map.reserve(2000000);
        result.string_pool.reserve(50000000);

        //3.循环获取数据
        while (DeviceIoControl(hVol, FSCTL_ENUM_USN_DATA, &mftEnumData, sizeof(mftEnumData),
                               buffer, BUFFER_SIZE, &byteCount, NULL)) {
            DWORDLONG nextUsn = *((DWORDLONG*)buffer);
            USN_RECORD* record = (USN_RECORD*)((BYTE*)buffer + sizeof(DWORDLONG));

            while ((BYTE*)record < buffer + byteCount) {
                Optimized_Node node;
                node.is_dir = (record->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                node.size = 0;

                WCHAR* fileNamePtr = (WCHAR*)((BYTE*)record + record->FileNameOffset);
                size_t nameLen = record->FileNameLength / sizeof(WCHAR);

                node.name_offset = (uint32_t)result.string_pool.size();
                node.name_len = (uint16_t)nameLen;

                result.string_pool.insert(result.string_pool.end(), fileNamePtr, fileNamePtr + nameLen);

                const DWORDLONG MFT_MASK = 0x0000FFFFFFFFFFFFULL;
                DWORDLONG pure_file_id = record->FileReferenceNumber & MFT_MASK;
                DWORDLONG pure_parent_id = record->ParentFileReferenceNumber & MFT_MASK;

                uint32_t currentIndex = (uint32_t)result.nodes.size();
                result.nodes.push_back(node);

                result.id_to_index.push_back({pure_file_id, currentIndex});
                result.parent_map.push_back({pure_parent_id, currentIndex});

                record = (USN_RECORD*)((BYTE*)record + record->RecordLength);
            }
            mftEnumData.StartFileReferenceNumber = nextUsn;
        }

        CloseHandle(hVol);

        if (!result.nodes.empty()) {
            result.success = true;
            return result;
        }

        // MFT 拿到了句柄但没数据 → 清空残留，走 fallback
        result.nodes.clear();
        result.id_to_index.clear();
        result.parent_map.clear();
        result.string_pool.clear();
    }  // if (hVol != INVALID_HANDLE_VALUE)

    }  // if (try_mft)

    // ── std::filesystem 递归遍历（FAT32 / exFAT 兜底）────────────────
    using namespace std::filesystem;
    std::error_code ec;

    std::wstring root;
    root += static_cast<wchar_t>(drive_letter);
    root += L":\\";

    if (!exists(root, ec) || !is_directory(root, ec)) {
        result.error_message = "Drive not accessible";
        return result;
    }

    // 根节点
    {
        Optimized_Node rn;
        rn.is_dir = 1; rn.size = 0;
        rn.name_offset = 0; rn.name_len = 0;
        result.nodes.push_back(rn);
    }
    uint32_t rootIdx = 0;
    result.id_to_index.push_back({5ULL, rootIdx});

    DWORDLONG nextFid = 1000;

    // 规范化 key → 节点下标
    std::unordered_map<std::wstring, uint32_t> dirMap;
    {
        std::wstring rk = root;
        for (auto& c : rk) c = static_cast<wchar_t>(::towlower(c));
        if (rk.back() != L'\\') rk += L'\\';
        dirMap[rk] = rootIdx;
    }

    for (recursive_directory_iterator it(root, directory_options::skip_permission_denied, ec), end;
         it != end && !ec; ) {
        const directory_entry& entry = *it;
        std::error_code e2;
        bool isDir  = entry.is_directory(e2);
        bool isFile = entry.is_regular_file(e2);
        if (!isDir && !isFile) { ++it; continue; }

        std::wstring full = entry.path().wstring();
        if (full == root) { ++it; continue; }

        std::wstring name = entry.path().filename().wstring();
        std::wstring parentPath = entry.path().parent_path().wstring();
        if (!parentPath.empty() && parentPath.back() != L'\\') parentPath += L'\\';

        // 找父节点
        std::wstring parentKey = parentPath;
        for (auto& c : parentKey) c = static_cast<wchar_t>(::towlower(c));
        if (parentKey.back() != L'\\') parentKey += L'\\';

        uint32_t parentIdx = rootIdx;
        auto pm = dirMap.find(parentKey);
        if (pm != dirMap.end()) parentIdx = pm->second;

        uint32_t curIdx = (uint32_t)result.nodes.size();

        DWORDLONG fid = nextFid++;
        result.id_to_index.push_back({fid, curIdx});
        result.parent_map.push_back({result.id_to_index[parentIdx].first, curIdx});

        if (isDir) {
            Optimized_Node dn;
            dn.is_dir = 1; dn.size = 0;
            dn.name_offset = (uint32_t)result.string_pool.size();
            dn.name_len    = (uint16_t)name.size();
            result.string_pool.insert(result.string_pool.end(), name.begin(), name.end());
            result.nodes.push_back(dn);

            // 缓存目录
            std::wstring selfKey = full;
            for (auto& c : selfKey) c = static_cast<wchar_t>(::towlower(c));
            if (selfKey.back() != L'\\') selfKey += L'\\';
            dirMap[selfKey] = curIdx;

            ++it;
        } else {
            Optimized_Node fn;
            fn.is_dir = 0;
            fn.size   = entry.file_size(e2);
            fn.name_offset = (uint32_t)result.string_pool.size();
            fn.name_len    = (uint16_t)name.size();
            result.string_pool.insert(result.string_pool.end(), name.begin(), name.end());
            try {
                auto ft = entry.last_write_time(e2);
                auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    ft.time_since_epoch()).count();
                fn.last_modified = (uint64_t)ns / 100 + 116444736000000000ULL;
            } catch (...) { fn.last_modified = 0; }
            result.nodes.push_back(fn);

            ++it;
        }
    }

    // 排序（build_tree 依赖二分查找）
    std::sort(result.id_to_index.begin(), result.id_to_index.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    result.success = true;
    return result;
}