#include "treemapengine.h"
#include <algorithm>
#include <cmath>
#include <limits>

// ── 公开入口 ──────────────────────────────────────────────────────────
std::vector<TreemapItem> TreemapEngine::compute(
    const std::vector<qint64>&   sizes,
    const std::vector<uint32_t>& indices,
    const std::vector<QString>&  names,
    const std::vector<bool>&     is_dirs,
    double rect_x, double rect_y,
    double rect_w, double rect_h,
    double exponent)
{
    const size_t n = sizes.size();
    std::vector<TreemapItem> result;
    result.reserve(n);

    if (n == 0) return result;
    if (rect_w <= 0.0 || rect_h <= 0.0) return result;

    // 1. 计算总字节数，构建内部条目
    qint64 total_bytes = 0;
    for (qint64 sz : sizes) total_bytes += sz;
    if (total_bytes <= 0) return result;

    std::vector<Entry> entries;
    entries.reserve(n);

    const double total_area = rect_w * rect_h;

    // ── size → 视觉面积的映射 ──
    // 先算所有 transformed 值，再归一化
    std::vector<double> transformed(n);
    double sum_transformed = 0.0;

    for (size_t i = 0; i < n; ++i) {
        if (sizes[i] <= 0) continue;
        double t;
        if (exponent <= 0.0) {
            t = 1.0;                       // exponent=0 → 等分
        } else if (exponent == 1.0) {
            t = static_cast<double>(sizes[i]);   // 线性，跳过 pow
        } else {
            t = std::pow(static_cast<double>(sizes[i]), exponent);
        }
        transformed[i] = t;
        sum_transformed += t;
    }

    if (sum_transformed <= 0.0) return result;

    for (size_t i = 0; i < n; ++i) {
        if (sizes[i] <= 0) continue;
        Entry e;
        e.index   = indices[i];
        e.name    = names[i];
        e.is_dir  = is_dirs[i];
        e.raw_size = sizes[i];                                        // 原始字节数
        e.norm_area = (transformed[i] / sum_transformed) * total_area; // 归一化视觉面积
        if (e.norm_area <= 0.0) e.norm_area = 1.0;                   // 兜底
        entries.push_back(e);
    }

    if (entries.empty()) return result;

    // 2. 按归一化面积降序排序（大块优先 → 方形化效果更好）
    std::sort(entries.begin(), entries.end(),
              [](const Entry& a, const Entry& b) { return a.norm_area > b.norm_area; });

    // 3. 递归布局
    squarify(entries, 0, rect_x, rect_y, rect_w, rect_h, result);
    return result;
}

// ── 计算一行（条）的最差长宽比 ───────────────────────────────────────
// row: 当前行的条目列表
// short_side: 行中所有条目共享的固定边长（矩形的短边）
// 返回 max( w_i/h_i, h_i/w_i )，返回值 ≥ 1，越小越接近正方形
double TreemapEngine::worst_ratio(
    const std::vector<Entry>& row,
    double short_side)
{
    if (short_side <= 0.0) return std::numeric_limits<double>::max();

    double worst = 0.0;
    const double s2 = short_side * short_side;

    for (const Entry& e : row) {
        double a = e.norm_area;
        // ratio = max( a/s², s²/a )
        double r = a / s2;
        if (r < 1.0) r = 1.0 / r;
        if (r > worst) worst = r;
    }
    return worst;
}

// ── 递归核心 ──────────────────────────────────────────────────────────
void TreemapEngine::squarify(
    std::vector<Entry>& entries,
    size_t start,
    double x, double y, double w, double h,
    std::vector<TreemapItem>& result)
{
    if (start >= entries.size()) return;
    if (w <= 0.0 || h <= 0.0) return;

    const size_t remaining = entries.size() - start;

    // 只剩一项：填满整个剩余矩形
    if (remaining == 1) {
        const Entry& e = entries[start];
        TreemapItem item;
        item.x = x;  item.y = y;
        item.w = w;  item.h = h;
        item.node_index   = e.index;
        item.name         = e.name;
        item.size         = e.raw_size;
        item.is_directory = e.is_dir;
        result.push_back(item);
        return;
    }

    // ── 确定布局方向 ──
    // 短边 = 所有条目共享的固定边长
    double short_side = (w >= h) ? h : w;

    // ── 贪心填行 ──
    size_t  row_end    = start;       // 行结束下标（不含）
    double  row_sum    = 0.0;         // 行内归一化面积累加
    double  prev_worst = std::numeric_limits<double>::max();

    for (size_t i = start; i < entries.size(); ++i) {
        // 临时行：当前行 + 新条目
        std::vector<Entry> temp_row(entries.begin() + start, entries.begin() + i + 1);
        double curr_worst = worst_ratio(temp_row, short_side);

        // 变差则回退（不加这一项）
        if (curr_worst >= prev_worst) {
            break;
        }

        prev_worst = curr_worst;
        row_sum   += entries[i].norm_area;
        row_end    = i + 1;
    }

    // 兜底：至少放一项
    if (row_end == start) {
        row_end = start + 1;
        row_sum = entries[start].norm_area;
    }

    // ── 放置该行 ──
    if (w >= h) {
        // 垂直条布局：行填满整个高度，各条目从左到右按面积比例分配宽度
        double row_width = (h > 0.0) ? (row_sum / h) : 0.0;
        double cx = x;  // 当前 x 光标

        for (size_t i = start; i < row_end; ++i) {
            const Entry& e = entries[i];
            double item_w = (h > 0.0) ? (e.norm_area / h) : 0.0;

            TreemapItem item;
            item.x = cx;   item.y = y;
            item.w = item_w; item.h = h;
            item.node_index   = e.index;
            item.name         = e.name;
            item.size         = e.raw_size;
            item.is_directory = e.is_dir;
            result.push_back(item);

            cx += item_w;
        }

        // 剩余矩形在右侧
        squarify(entries, row_end, x + row_width, y, w - row_width, h, result);
    } else {
        // 水平条布局：行填满整个宽度，各条目从上到下按面积比例分配高度
        double row_height = (w > 0.0) ? (row_sum / w) : 0.0;
        double cy = y;  // 当前 y 光标

        for (size_t i = start; i < row_end; ++i) {
            const Entry& e = entries[i];
            double item_h = (w > 0.0) ? (e.norm_area / w) : 0.0;

            TreemapItem item;
            item.x = x;  item.y = cy;
            item.w = w;  item.h = item_h;
            item.node_index   = e.index;
            item.name         = e.name;
            item.size         = e.raw_size;
            item.is_directory = e.is_dir;
            result.push_back(item);

            cy += item_h;
        }

        // 剩余矩形在下方
        squarify(entries, row_end, x, y + row_height, w, h - row_height, result);
    }
}
