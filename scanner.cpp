#include "scanner.h"
#include <winioctl.h>

#define BUFFER_SIZE (1024 * 64)

scanner::Scan_Result scanner::scan_drive(char drive_letter) {
    Scan_Result result;

    //1.组装路径
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

    if (hVol == INVALID_HANDLE_VALUE) {
        result.error_message = "Failed to open volume";
        return result;
    }

    //2.初始化MFT参数
    MFT_ENUM_DATA mftEnumData;
    mftEnumData.StartFileReferenceNumber = 0;  //第一个文件
    mftEnumData.LowUsn = 0;
    mftEnumData.HighUsn = MAXLONGLONG;             //查询所有文件

    BYTE buffer[BUFFER_SIZE];
    DWORD byteCount = 0;

    //预分配空间
    result.nodes.reserve(2000000);
    result.id_to_index.reserve(2000000);
    result.parent_map.reserve(2000000);
    result.string_pool.reserve(50000000);

    //3.循环获取数据
    while (DeviceIoControl(hVol, FSCTL_ENUM_USN_DATA, &mftEnumData, sizeof(mftEnumData), buffer, BUFFER_SIZE, &byteCount, NULL)) {
        DWORDLONG nextUsn = *((DWORDLONG*)buffer);
        USN_RECORD* record = (USN_RECORD*)((BYTE*)buffer + sizeof(DWORDLONG));

        while ((BYTE*)record < buffer + byteCount) {
            Optimized_Node node;

            //提取基础属性
            node.is_dir = (record->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            node.size = 0;

            //处理字符串池
            WCHAR* fileNamePtr = (WCHAR*)((BYTE*)record + record->FileNameOffset);
            size_t nameLen = record->FileNameLength / sizeof(WCHAR);

            node.name_offset = (uint32_t)result.string_pool.size();
            node.name_len = (uint16_t)nameLen;

            //将文件名塞入连续内存池
            result.string_pool.insert(result.string_pool.end(), fileNamePtr, fileNamePtr + nameLen);

            const DWORDLONG MFT_MASK = 0x0000FFFFFFFFFFFFULL;

            DWORDLONG pure_file_id = record->FileReferenceNumber & MFT_MASK;
            DWORDLONG pure_parent_id = record->ParentFileReferenceNumber & MFT_MASK;

            //记录当前节点在数组中的索引
            uint32_t currentIndex = (uint32_t)result.nodes.size();
            result.nodes.push_back(node);

            //存入配对表
            result.id_to_index.push_back({pure_file_id, currentIndex});
            result.parent_map.push_back({pure_parent_id, currentIndex});

            record = (USN_RECORD*)((BYTE*)record + record->RecordLength);
        }
        mftEnumData.StartFileReferenceNumber = nextUsn;
    }

    CloseHandle(hVol);
    result.success = true;
    return result;
}