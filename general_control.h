#ifndef GENERAL_CONTROL_H
#define GENERAL_CONTROL_H

#include <QObject>
#include <QString>
#include <QList>
#include <QHash>
#include <QFutureWatcher>
#include <windows.h>
#include "datatype.h"
#include "scanner.h"
#include "filemanager.h"
#include "UsbDeviceMonitor.h"
#include "treemapengine.h"

//并发任务结构体
struct file_size_task {
    uint32_t node_idx;
    QString absolute_path;
};

//盘隔离
struct drive_content {
    std::vector<Optimized_Node> memory_tree;
    std::vector<WCHAR> string_pool;
    uint32_t root_idx = INVALID_INDEX;
    bool is_scanned = false;
};

//后台任务
struct scan_task_result {
    bool success = false;
    QString drive_letter;
    QString error_message;
    drive_content ctx;
};

class general_control : public QObject {
    Q_OBJECT
public:
    explicit general_control(QObject *parent = nullptr);
    ~general_control();

    //启动扫描，输入C、D、E即可；refresh参数决定是否对已扫描过的磁盘重新扫描
    void start_scan(const QString& drive_letter, bool refresh = false);

    //获取打包内容
    QList<UI_Block> get_content(const QString& drive_letter, uint32_t target_index);

    //关键词搜索
    QList<UI_Block> search_file(const QString& drive_letter, uint32_t target_index, const QString& key_words);

    // Squarified Treemap：输入盘符+目录节点+矩形尺寸 → 输出每个子项的 x/y/w/h
    // exponent 控制大小压缩：1.0=线性 0.35=推荐 0.0=等分
    std::vector<TreemapItem> get_treemap(const QString& drive_letter,
                                          uint32_t target_index,
                                          double rect_w, double rect_h,
                                          double exponent = 1.0) const;

    //idx->绝对路径
    QString get_absolute_path(const QString& drive_letter, uint32_t node_index);

    //idx->name
    QString get_node_name(const QString& drive_letter, uint32_t node_index);

    //直接获取idx的内容
    UI_Block get_target_content(const QString& drive_letter, uint32_t target_index);

    //获取文件路径链
    QList<file_location> get_path_chain(const file_location& target);

    //文件操作函数
    bool renameFile(const file_location& target, const QString& new_name);
    bool deleteFile(const QList<file_location>& targets);
    bool setClipboard(const QList<file_location>& targets, filemanager::clipboard_operation operation);
    bool execute_paste(const file_location& destination_folder);
    bool change_file_extension(const file_location& target_file, const QString& new_extention);

signals:
    //扫盘相关信号
    void scan_started(const QString& drive_letter);
    void scan_finished(const QString& drive_letter, uint32_t root_index);
    void scan_error(const QString& drive_letter, const QString& error_message);

    //文件操作相关信号
    void rename_finished();
    void delete_finished();
    void paste_finished();

    //U盘热插拔信号
    void usb_device_changed();

    //统一报错信号
    void operation_error(const QString& action_name, const QString& error_message);

private slots:
    void scan_thread_finish();

private:
    //建树相关
    void build_tree(drive_content& ctx, scanner::Scan_Result& raw_data, const QString& drive_letter);
    uint64_t caculate_size(drive_content& ctx, uint32_t node_idx);
    void collect_task(drive_content& ctx, uint32_t node_idx, const QString& current_path, QList<file_size_task>& task_list);

    //搜索辅助
    void search_helper(const QString& drive_letter, drive_content& ctx, uint32_t node_idx, const QString& key_words, QList<UI_Block>& result, int max_results);

    //内存同步函数
    void update_memory_after_delete(drive_content& ctx, uint32_t target_idx);
    void update_memory_after_rename(drive_content& ctx, uint32_t target_idx, const QString& new_name);
    uint32_t update_memory_after_copy(drive_content& ctx, uint32_t source_idx, uint32_t new_parent_idx, const QString& optional_new_name);

    //向上更新大小辅助函数
    void update_size(drive_content& ctx, uint32_t start_node_idx, qint64 size_delta, int imm_file_delta, int total_file_delta);
    void detach_node(drive_content& ctx, uint32_t target_idx);
    void attach_node(drive_content& ctx, uint32_t target_idx, uint32_t new_parent_idx);

    //核心数据
    QHash<QString, drive_content> drive_map;

    //管理异步任务
    QFutureWatcher<scan_task_result> scan_watcher;

    //U盘热插拔监听
    UsbDeviceMonitor m_usbMonitor;

    //物理修改器实例化
    filemanager file_worker;
    QList<file_location> m_clipboard_targets;
    filemanager::clipboard_operation m_current_op = filemanager::clipboard_operation::None;
};

#endif // GENERAL_CONTROL_H