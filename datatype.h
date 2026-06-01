//基础数据结构类
#ifndef DATATYPE_H
#define DATATYPE_H

#include <QString>
#include <QDateTime>
#include <cstdint>
#include <windows.h>

#define INVALID_INDEX 0xFFFFFFFF

#pragma pack(push, 1)

//搜索节点
struct Optimized_Node {
    uint32_t parent_index = INVALID_INDEX;   //父节点
    uint32_t first_child = INVALID_INDEX;    //子节点
    uint32_t next_sibling = INVALID_INDEX;   //兄弟节点
    uint64_t size : 48;                      //文件大小
    uint64_t is_dir : 1;                     //是否是文件夹
    uint32_t name_offset = 0;                //在stringpool中的起始位置
    uint16_t name_len = 0;                   //文件名长度
    uint32_t total_file = 0;                  //仅文件夹：文件总数
    uint32_t immediate_file = 0;              //仅文件夹：一级子目录文件数
    uint64_t last_modified = 0;               //修改时间
};

#pragma pack(pop)

//传递节点
struct UI_Block {
    QString file_name;                        //文件名
    qint64 size;                              //文件大小
    bool is_directory;                        //是否是文件夹
    uint32_t total_file;                      //仅文件夹：文件总数
    uint32_t immediate_file;                  //仅文件夹：一级子目录文件数
    QDateTime last_modified;                  //修改时间
    uint32_t file_index;                      //对应搜索节点idx
    QString absolute_path;                    //绝对路径
};

//定位器
struct file_location {
    QString drive;                            //盘符
    uint32_t index;                           //对应搜索节点idx
};

#endif // DATATYPE_H