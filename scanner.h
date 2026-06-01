#ifndef SCANNER_H
#define SCANNER_H

#include <vector>
#include <string>
#include <windows.h>
#include "datatype.h"

class scanner
{
public:
    //扫描结果
    struct Scan_Result {
        std::vector<Optimized_Node> nodes;
        //配对数组1：原始64位FileID,对应的node数组下标
        std::vector<std::pair<DWORDLONG, uint32_t>> id_to_index;
        //配对数组2：原始64位ParentID,对应的node数组下标
        std::vector<std::pair<DWORDLONG, uint32_t>> parent_map;
        //文件名池
        std::vector<WCHAR> string_pool;
        bool success = false;
        std::string error_message;
    };

    //传入盘符
    static Scan_Result scan_drive(char drive_letter);
};

#endif // SCANNER_H
