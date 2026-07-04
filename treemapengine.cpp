#include "treemapengine.h"
#include <algorithm>
#include <cmath>

// ── 公开入口 ──────────────────────────────────────────────────────────
std::vector<TreemapItem> TreemapEngine::compute(
    const std::vector<qint64>&   sizes,
    const std::vector<uint32_t>& indices,
    const std::vector<QString>&  names,
    const std::vector<bool>&     is_dirs,
    double rect_x, double rect_y,
    double rect_w, double rect_h,
    double exponent,
    double floorRatio)
{
    const size_t n = sizes.size();
    std::vector<TreemapItem> result;
    result.reserve(n);

    if (n == 0) return result;
    if (rect_w <= 0.0 || rect_h <= 0.0) return result;

    // 1. 先算所有 pow 值，同时记录最大值
    std::vector<double> pows(n);
    double max_pow = 0.0;

    for (size_t i = 0; i < n; ++i) {
        if (sizes[i] <= 0) continue;
        double t;
        if (exponent <= 0.0) {
            t = 1.0;
        } else if (exponent == 1.0) {
            t = static_cast<double>(sizes[i]);
        } else {
            t = std::pow(static_cast<double>(sizes[i]), exponent);
        }
        pows[i] = t;
        if (t > max_pow) max_pow = t;
    }

    // 2. 底薪 = 最大的 pow × floorRatio；提成 = pow
    //    transformed = floor + pow   （保底 + 按比例）
    double floor = max_pow * floorRatio;
    std::vector<double> transformed(n);
    double sum_transformed = 0.0;

    for (size_t i = 0; i < n; ++i) {
        if (sizes[i] <= 0) continue;
        transformed[i] = floor + pows[i];
        sum_transformed += transformed[i];
    }

    if (sum_transformed <= 0.0) return result;

    std::vector<Entry> entries;
    entries.reserve(n);

    const double total_area = rect_w * rect_h;

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

    // 3. 按归一化面积降序排序
    std::sort(entries.begin(), entries.end(),
              [](const Entry& a, const Entry& b) { return a.norm_area > b.norm_area; });

    // 4. 找分割点：累计面积 85% 以上的大文件走 pivot，剩余小文件走 grid
    double total = 0.0;
    for (const auto& e : entries) total += e.norm_area;

    double cumulative = 0.0;
    size_t split = entries.size();
    for (size_t i = 0; i < entries.size(); ++i) {
        cumulative += entries[i].norm_area;
        if (cumulative >= total * 0.85) {
            split = i + 1;
            break;
        }
    }
    if (split > entries.size()) split = entries.size();

    // 5a. 大文件 → pivot，按面积比例分配上部
    if (split > 0) {
        double pivot_area = 0.0;
        for (size_t i = 0; i < split; ++i) pivot_area += entries[i].norm_area;
        double pivot_h = (pivot_area / total) * rect_h;
        pivot_layout(entries, 0, split, rect_x, rect_y, rect_w, pivot_h, result);
    }

    // 5b. 小文件 → 网格，剩余下部
    if (split < entries.size()) {
        double pivot_area = 0.0;
        for (size_t i = 0; i < split; ++i) pivot_area += entries[i].norm_area;
        double pivot_h = (pivot_area / total) * rect_h;
        double grid_y = rect_y + pivot_h;
        double grid_h = rect_h - pivot_h;
        tail_grid(entries, split, entries.size(), rect_x, grid_y, rect_w, grid_h, result);
    }

    return result;
}

// ── Pivot 递归二分 ────────────────────────────────────────────────────
// 每次把 items 按面积分成近似的两半，沿长边切分矩形
void TreemapEngine::pivot_layout(
    std::vector<Entry>& entries,
    size_t start, size_t end,
    double x, double y, double w, double h,
    std::vector<TreemapItem>& result)
{
    if (start >= end) return;
    if (w <= 0.0 || h <= 0.0) return;

    if (end - start == 1) {
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

    // ── 找 pivot：总面积的中间点 ──
    double total = 0.0;
    for (size_t i = start; i < end; ++i) total += entries[i].norm_area;
    double half = total * 0.5;
    double cum = 0.0;
    size_t pivot = start;
    while (pivot < end && cum + entries[pivot].norm_area <= half) {
        cum += entries[pivot].norm_area;
        ++pivot;
    }
    // 边界：至少一个 item 在左半
    if (pivot == start) { cum += entries[start].norm_area; pivot = start + 1; }
    if (pivot == end)   { pivot = end - 1; }

    double left_area = cum;
    double right_area = total - left_area;

    // ── 沿长边切分 ──
    if (w >= h) {
        double left_w  = (left_area / total) * w;
        double right_w = w - left_w;
        pivot_layout(entries, start, pivot, x, y, left_w, h, result);
        pivot_layout(entries, pivot, end, x + left_w, y, right_w, h, result);
    } else {
        double top_h    = (left_area / total) * h;
        double bottom_h = h - top_h;
        pivot_layout(entries, start, pivot, x, y, w, top_h, result);
        pivot_layout(entries, pivot, end, x, y + top_h, w, bottom_h, result);
    }
}

// ── 小文件网格 ────────────────────────────────────────────────────────
// 等分网格，保证每个 item 接近正方形
void TreemapEngine::tail_grid(
    std::vector<Entry>& entries,
    size_t start, size_t end,
    double x, double y, double w, double h,
    std::vector<TreemapItem>& result)
{
    size_t count = end - start;
    if (count == 0) return;

    // 算格子数以接近正方形为目标
    int cols = std::max(1, (int)std::ceil(std::sqrt(count * w / h)));
    int rows = std::max(1, (int)std::ceil((double)count / (double)cols));

    double cell_w = w / cols;
    double cell_h = h / rows;

    for (size_t i = start; i < end; ++i) {
        const Entry& e = entries[i];
        int idx = (int)(i - start);
        int col = idx % cols;
        int row = idx / cols;

        // 最后一行不满 → 拉伸每项宽度填满
        size_t row_start = start + row * cols;
        size_t row_end   = std::min(end, row_start + cols);
        size_t in_row    = row_end - row_start;
        double item_w    = (in_row == (size_t)cols) ? cell_w : (w / in_row);

        TreemapItem item;
        item.x = x + col * item_w;
        item.y = y + row * cell_h;
        item.w = item_w;
        item.h = cell_h;
        item.node_index   = e.index;
        item.name         = e.name;
        item.size         = e.raw_size;
        item.is_directory = e.is_dir;
        result.push_back(item);
    }
}