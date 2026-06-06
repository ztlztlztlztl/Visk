#ifndef TREEMAPENGINE_H
#define TREEMAPENGINE_H

#include "datatype.h"
#include <vector>
#include <cstdint>
#include <QString>

// ── Treemap 输出：一个矩形块 ──────────────────────────────────────────
struct TreemapItem {
    uint32_t node_index = 0;   // 对应 memory_tree 中的节点索引
    QString  name;             // 文件/目录名
    qint64   size = 0;         // 字节数
    bool     is_directory = false;
    double   x = 0, y = 0;     // 左上角坐标 (像素或归一化)
    double   w = 0, h = 0;     // 宽高
};

// ── Squarified Treemap 引擎（纯计算，无 Qt 依赖） ───────────────────
class TreemapEngine {
public:
    // 一次性计算：输入一组 (size, index, name, isDir)，输出带坐标的矩形列表
    // sizes 会被内部排序（降序），输出顺序 = 面积大的在前
    // (x,y,w,h) 初始矩形区域 — 可以用像素值，也可以用 1.0×1.0 归一化
    // exponent 控制大小差距压缩度：
    //   1.0  线性（原始比例）；0.5 sqrt（推荐）；0.0 等分
    // floorRatio 小文件保底比例：
    //   transformed = pow(size, exponent) + max_pow * floorRatio
    //   0.0 不保底；0.005 推荐
    static std::vector<TreemapItem> compute(
        const std::vector<qint64>&   sizes,
        const std::vector<uint32_t>& indices,
        const std::vector<QString>&  names,
        const std::vector<bool>&     is_dirs,
        double rect_x, double rect_y,
        double rect_w, double rect_h,
        double exponent   = 0.5,
        double floorRatio = 0.005);

private:
    // 内部带排序的条目
    struct Entry {
        qint64   raw_size;    // 原始字节数（输出用）
        double   norm_area;   // 归一化视觉面积（布局用）
        uint32_t index;
        QString  name;
        bool     is_dir;
    };

    // 递归核心
    static void squarify(
        std::vector<Entry>& entries,
        size_t start,                     // 当前处理的起始下标
        double x, double y, double w, double h,
        std::vector<TreemapItem>& result);

    // 计算把 row 放在长度为 row_length 的条中的最差长宽比
    static double worst_ratio(
        const std::vector<Entry>& row,
        double row_length);
};

#endif // TREEMAPENGINE_H
