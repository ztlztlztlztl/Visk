#include "general_control.h"
#include <QtConcurrent>
#include <algorithm>
#include <QDebug>

general_control::general_control(QObject *parent) : QObject(parent) {
    //将后台线程的完成信号连接到主线程的槽函数
    connect(&scan_watcher, &QFutureWatcher<scanner::Scan_Result>::finished,
            this, &general_control::scan_thread_finish);
}

general_control::~general_control() {
    if (scan_watcher.isRunning()) {
        scan_watcher.cancel();
        scan_watcher.waitForFinished();
    }
}

void general_control::start_scan(const QString& drive_letter, bool refresh) {
    if (scan_watcher.isRunning()) return;

    if (drive_letter.isEmpty()) return;

    QString current_drive = drive_letter.toUpper();

    if (!refresh && drive_map.contains(current_drive) && drive_map[current_drive].is_scanned) {
        emit scan_finished(current_drive, drive_map[current_drive].root_idx);
        return;
    }

    char target_drive = current_drive.toStdString()[0];

    emit scan_started(current_drive);

    //全局线程池
    QFuture<scan_task_result> future = QtConcurrent::run([this, target_drive, current_drive]() {
        scan_task_result res;
        res.drive_letter = current_drive;

        //底层MFT扫描
        scanner::Scan_Result raw_data = scanner::scan_drive(target_drive);
        res.success = raw_data.success;

        if (!raw_data.success) {
            res.error_message = QString::fromStdString(raw_data.error_message);
            return res;
        }

        //将字符串池转移到临时的上下文ctx中
        res.ctx.string_pool = std::move(raw_data.string_pool);

        //直接在后台线程建树
        this->build_tree(res.ctx, raw_data, current_drive);

        res.ctx.is_scanned = true;
        return res;
    });

    scan_watcher.setFuture(future);
}

void general_control::scan_thread_finish() {
    scan_task_result result = scan_watcher.result();

    if (!result.success) {
        emit scan_error(result.drive_letter, result.error_message);
        return;
    }

    drive_map[result.drive_letter] = std::move(result.ctx);

    //通知UI渲染
    emit scan_finished(result.drive_letter, drive_map[result.drive_letter].root_idx);
}

void general_control::build_tree(drive_content& ctx, scanner::Scan_Result& raw_data, const QString& drive_letter) {
    ctx.memory_tree = std::move(raw_data.nodes);

    bool has_root = false;
    for (const auto& pair : raw_data.id_to_index) {
        if (pair.first == 5ULL) {
            has_root = true;
            break;
        }
    }

    if (!has_root) {
        Optimized_Node fake_root;
        fake_root.is_dir = 1;
        fake_root.size = 0;
        fake_root.name_offset = 0;  //名字为空,不影响UI绝对路径的生成
        fake_root.name_len = 0;

        // 将虚拟根节点推入树中
        uint32_t fake_root_idx = (uint32_t)ctx.memory_tree.size();
        ctx.memory_tree.push_back(fake_root);

        // 强行把 5ULL 塞进索引表
        raw_data.id_to_index.push_back({5ULL, fake_root_idx});
    }

    //对FileId进行排序
    std::sort(raw_data.id_to_index.begin(), raw_data.id_to_index.end(),
        [](const auto& a, const auto& b) {
        return a.first < b.first;
    });

    //将子节点挂在父节点上
    for (const auto& relation : raw_data.parent_map) {
        DWORDLONG parent_id = relation.first;
        uint32_t child_idx = relation.second;

        auto it = std::lower_bound(raw_data.id_to_index.begin(), raw_data.id_to_index.end(), parent_id,
            [](const auto& pair, DWORDLONG val) {
            return pair.first < val;
        });

        if (it != raw_data.id_to_index.end() && it->first == parent_id) {
            uint32_t parent_idx = it->second;

            if (parent_idx == child_idx) continue;

            ctx.memory_tree[child_idx].next_sibling = ctx.memory_tree[parent_idx].first_child;
            ctx.memory_tree[parent_idx].first_child = child_idx;
            ctx.memory_tree[child_idx].parent_index = parent_idx;

            if (!ctx.memory_tree[child_idx].is_dir) {
                ctx.memory_tree[parent_idx].immediate_file++;
            }
        }
    }

    //寻找根目录（一般id是5）
    auto root_it = std::lower_bound(raw_data.id_to_index.begin(), raw_data.id_to_index.end(), 5ULL,
        [](const auto& pair, DWORDLONG val) {
        return pair.first < val;
    });

    if (root_it != raw_data.id_to_index.end() && root_it->first == 5ULL) {
        ctx.root_idx = root_it->second;
    }
    else {
        //容错：随便找没有父节点的当树根
        for (uint32_t i = 0; i < ctx.memory_tree.size(); ++i) {
            if (ctx.memory_tree[i].parent_index == INVALID_INDEX) {
                ctx.root_idx = i;
                break;
            }
        }
    }

    if (ctx.root_idx == INVALID_INDEX) return;

    //收集文件路径
    QList<file_size_task> tasks;
    uint32_t child_idx = ctx.memory_tree[ctx.root_idx].first_child;
    while (child_idx != INVALID_INDEX) {
        collect_task(ctx, child_idx, drive_letter + ":\\", tasks);
        child_idx = ctx.memory_tree[child_idx].next_sibling;
    }

    //多线程处理文件任务
    QtConcurrent::blockingMap(tasks, [&ctx](const file_size_task& task) {
        WIN32_FILE_ATTRIBUTE_DATA file_info;

        if (GetFileAttributesEx((LPCWSTR)task.absolute_path.utf16(), GetFileExInfoStandard, &file_info)) {
            uint64_t exact_size = ((uint64_t)file_info.nFileSizeHigh << 32) | file_info.nFileSizeLow;
            ctx.memory_tree[task.node_idx].size = exact_size;

            uint64_t mod_time = ((uint64_t)file_info.ftLastWriteTime.dwHighDateTime << 32) | file_info.ftLastWriteTime.dwLowDateTime;

            ctx.memory_tree[task.node_idx].last_modified = mod_time;
        }
    });

    caculate_size(ctx, ctx.root_idx);
}

uint64_t general_control::caculate_size(drive_content& ctx, uint32_t node_idx) {
    if (node_idx == INVALID_INDEX || node_idx >= ctx.memory_tree.size()) return 0;
    Optimized_Node& node = ctx.memory_tree[node_idx];

    if (!node.is_dir) return node.size;

    //遍历子文件
    uint64_t total_size = 0;
    uint32_t total_cnt = node.immediate_file;
    uint32_t child_idx = node.first_child;

    while (child_idx != INVALID_INDEX) {
        total_size += caculate_size(ctx, child_idx);

        if (ctx.memory_tree[child_idx].is_dir) {
            total_cnt += ctx.memory_tree[child_idx].total_file;
        }

        child_idx = ctx.memory_tree[child_idx].next_sibling;
    }

    node.size = total_size;
    node.total_file = total_cnt;

    return total_size;
}

void general_control::collect_task(drive_content& ctx, uint32_t node_idx, const QString& current_path, QList<file_size_task>& task_list) {
    if (node_idx == INVALID_INDEX) return;

    Optimized_Node& node = ctx.memory_tree[node_idx];

    //提取文件名
    QString file_name = QString::fromWCharArray(&ctx.string_pool[node.name_offset], node.name_len);

    //拼装路径
    QString full_path;
    if (current_path.endsWith('\\')) {
        full_path = current_path + file_name;
    }
    else {
        full_path = current_path + "\\" + file_name;
    }

    if (!node.is_dir) {
        task_list.append({node_idx, full_path});
    }
    else {
        uint32_t child_idx = node.first_child;
        while(child_idx != INVALID_INDEX) {
            collect_task(ctx, child_idx, full_path, task_list);
            child_idx = ctx.memory_tree[child_idx].next_sibling;
        }
    }
}

QString general_control::get_node_name(const QString& drive_letter, uint32_t node_index) {
    if (!drive_map.contains(drive_letter)) return "";
    const drive_content& ctx = drive_map[drive_letter];

    if (node_index == INVALID_INDEX || node_index >= ctx.string_pool.size()) return "";
    const Optimized_Node& node = ctx.memory_tree[node_index];
    return QString::fromWCharArray(&ctx.string_pool[node.name_offset], node.name_len);
}


QList<UI_Block> general_control::get_content(const QString& drive_letter, uint32_t target_index) {
    QList<UI_Block> result;
    if (!drive_map.contains(drive_letter)) return result;

    const drive_content& ctx = drive_map[drive_letter];
    if (target_index == INVALID_INDEX || target_index >= ctx.memory_tree.size()) return result;

    const Optimized_Node& parent_node = ctx.memory_tree[target_index];
    uint32_t child_idx = parent_node.first_child;

    //把子节点拎出来
    while (child_idx != INVALID_INDEX) {
        const Optimized_Node& child_node = ctx.memory_tree[child_idx];

        UI_Block block;
        block.file_name = get_node_name(drive_letter, child_idx);
        block.size = child_node.size;
        block.is_directory = child_node.is_dir;
        block.total_file = child_node.total_file;
        block.immediate_file = child_node.immediate_file;
        block.file_index = child_idx;
        block.parent_index = child_node.parent_index;

        //时间解析
        if (child_node.last_modified != 0) {
            qint64 msecsSinceEpoch = (child_node.last_modified - 116444736000000000ULL) / 10000;
            block.last_modified = QDateTime::fromMSecsSinceEpoch(msecsSinceEpoch);
        }

        result.append(block);

        child_idx = child_node.next_sibling;
    }

    return result;
}

QString general_control::get_absolute_path(const QString& drive_letter, uint32_t node_index) {
    if (!drive_map.contains(drive_letter) || node_index == INVALID_INDEX) return "";

    const drive_content& ctx = drive_map[drive_letter];

    QStringList path_part;
    uint32_t current_idx = node_index;

    while(current_idx != INVALID_INDEX) {
        if (current_idx != INVALID_INDEX) path_part.prepend(get_node_name(drive_letter, current_idx));
        current_idx = ctx.memory_tree[current_idx].parent_index;
    }

    return drive_letter + ":\\" + path_part.join("\\");
}

QList<UI_Block> general_control::search_file(const QString& drive_letter, uint32_t target_index, const QString& key_words) {
    QList<UI_Block> result;

    if (key_words.isEmpty() || !drive_map.contains(drive_letter)) return result;

    drive_content& ctx =drive_map[drive_letter];
    if (target_index == INVALID_INDEX || target_index >= ctx.memory_tree.size()) return result;

    //忽略大小写
    QString lower_key = key_words.toLower();

    uint32_t child_idx = ctx.memory_tree[target_index].first_child;

    while (child_idx != INVALID_INDEX) {
        search_helper(drive_letter, ctx, child_idx, lower_key, result);
        child_idx = ctx.memory_tree[child_idx].next_sibling;
    }

    return result;
}

void general_control::search_helper(const QString& drive_letter, drive_content& ctx, uint32_t node_idx, const QString& key_words, QList<UI_Block>& result) {
    if (node_idx == INVALID_INDEX) return;

    const Optimized_Node& node = ctx.memory_tree[node_idx];
    QString node_name = QString::fromWCharArray(&ctx.string_pool[node.name_offset], node.name_len);

    //进行字串匹配
    if (node_name.toLower().contains(key_words)) {
        UI_Block block;

        block.file_name = node_name;
        block.size = node.size;
        block.is_directory = node.is_dir;
        block.total_file = node.total_file;
        block.immediate_file = node.immediate_file;
        block.file_index = node_idx;
        block.absolute_path = get_absolute_path(drive_letter, node_idx);

        result.append(block);
    }

    //继续递归孩子
    uint32_t child_idx = node.first_child;
    while(child_idx != INVALID_INDEX) {
        search_helper(drive_letter, ctx, child_idx, key_words, result);
    }
}

bool general_control::deleteFile(const QList<file_location>& targets) {
    bool all_success = true;

    for (const file_location& location : std::as_const(targets)) {
        if (!drive_map.contains(location.drive)) continue;
        //获取绝对路径
        drive_content& ctx =drive_map[location.drive];
        QString absolute_path = get_absolute_path(location.drive, location.index);
        if (file_worker.delete_file(absolute_path)) {
            update_memory_after_delete(ctx, location.index);
        }
        else {
            all_success = false;
        }
    }

    emit delete_finished();
    return all_success;
}

bool general_control::renameFile(const file_location& target, const QString& new_name) {
    if (!drive_map.contains(target.drive)) return false;

    drive_content& ctx = drive_map[target.drive];
    QString old_path = get_absolute_path(target.drive, target.index);

    if (file_worker.rename_file(old_path, new_name)) {
        std::wstring new_name_w = new_name.toStdWString();

        Optimized_Node& node = ctx.memory_tree[target.index];
        node.name_offset = (uint32_t)ctx.string_pool.size();
        node.name_len = (uint16_t)new_name_w.length();

        ctx.string_pool.insert(ctx.string_pool.end(), new_name_w.begin(), new_name_w.end());

        return true;
    }

    emit rename_finished();
    return false;
}

void general_control::update_memory_after_delete(drive_content& ctx, uint32_t target_idx) {
    if (target_idx == INVALID_INDEX) return;

    Optimized_Node& target = ctx.memory_tree[target_idx];
    uint32_t parent_idx = target.parent_index;

    //根节点不能删
    if (parent_idx == INVALID_INDEX) return;

    Optimized_Node& parent = ctx.memory_tree[parent_idx];

    //链表指针断开重连
    if (parent.first_child == target_idx) {
        parent.first_child = target.next_sibling;
    }
    else {
        uint32_t prev_sibling = parent.first_child;
        while (prev_sibling != INVALID_INDEX && ctx.memory_tree[prev_sibling].next_sibling!= target_idx) {
            prev_sibling = ctx.memory_tree[prev_sibling].next_sibling;
        }

        if (prev_sibling != INVALID_INDEX) {
            ctx.memory_tree[prev_sibling].next_sibling = target.next_sibling;
        }
    }

    //向上级汇报size扣减
    qint64 reduced_size = -((qint64)target.size);
    int imm_file_reduce = target.is_dir ? 0 : -1;
    int total_file_reduce = target.is_dir ? -((int)target.total_file) : -1;

    update_size(ctx, parent_idx, reduced_size, imm_file_reduce, total_file_reduce);

    //标记为废弃
    target.parent_index = INVALID_INDEX;
    target.next_sibling = INVALID_INDEX;
}

void general_control::update_size(drive_content& ctx, uint32_t start_node_idx, qint64 size_delta, int imm_file_delta, int total_file_delta) {
    uint32_t current_idx = start_node_idx;
    bool is_first = true;

    while(current_idx != INVALID_INDEX) {
        Optimized_Node& node = ctx.memory_tree[current_idx];

        node.size += size_delta;
        node.total_file += total_file_delta;

        if (is_first) {
            node.immediate_file += imm_file_delta;
            is_first = false;
        }

        current_idx = node.parent_index;
    }
}

void general_control::attach_node(drive_content& ctx, uint32_t target_idx, uint32_t new_parent_idx) {
    if (target_idx == INVALID_INDEX || new_parent_idx == INVALID_INDEX) return;

    Optimized_Node& target = ctx.memory_tree[target_idx];
    Optimized_Node& new_parent = ctx.memory_tree[new_parent_idx];

    //头插法连入新家庭
    target.parent_index = new_parent_idx;
    target.next_sibling = new_parent.first_child;
    new_parent.first_child = target_idx;

    //向上增加容量
    qint64 size_to_add = (qint64)target.size;
    int imm_file_add = target.is_dir ? 0 : 1;
    int total_file_add = target.is_dir ? (int)target.total_file : 1;
    update_size(ctx, new_parent_idx, size_to_add, imm_file_add, total_file_add);
}

void general_control::detach_node(drive_content& ctx, uint32_t target_idx) {
    if (target_idx == INVALID_INDEX) return;

    Optimized_Node& target = ctx.memory_tree[target_idx];
    uint32_t parent_idx = target.parent_index;
    if (parent_idx == INVALID_INDEX) return;

    Optimized_Node& parent = ctx.memory_tree[parent_idx];

    //链表断开重连
    if (parent.first_child == target_idx) {
        parent.first_child = target.next_sibling;
    } else {
        uint32_t prev_sibling = parent.first_child;
        while (prev_sibling != INVALID_INDEX && ctx.memory_tree[prev_sibling].next_sibling != target_idx) {
            prev_sibling = ctx.memory_tree[prev_sibling].next_sibling;
        }
        if (prev_sibling != INVALID_INDEX) {
            ctx.memory_tree[prev_sibling].next_sibling = target.next_sibling;
        }
    }

    //向上扣减容量
    qint64 size_to_reduce = -((qint64)target.size);
    int imm_file_reduce = target.is_dir ? 0 : -1;
    int total_file_reduce = target.is_dir ? -((int)target.total_file) : -1;
    update_size(ctx, parent_idx, size_to_reduce, imm_file_reduce, total_file_reduce);
}

uint32_t general_control::update_memory_after_copy(drive_content& ctx, uint32_t source_idx, uint32_t new_parent_idx, const QString& optional_new_name) {
    if (source_idx == INVALID_INDEX) return INVALID_INDEX;

    //值拷贝基础属性
    Optimized_Node new_node = ctx.memory_tree[source_idx];
    new_node.parent_index = new_parent_idx;
    new_node.first_child = INVALID_INDEX;
    new_node.next_sibling = INVALID_INDEX;

    //如果是顶层文件有重名冲突，使用新名字
    if (!optional_new_name.isEmpty()) {
        std::wstring new_name_w = optional_new_name.toStdWString();
        new_node.name_offset = (uint32_t)ctx.string_pool.size();
        new_node.name_len = (uint16_t)new_name_w.length();
        ctx.string_pool.insert(ctx.string_pool.end(), new_name_w.begin(), new_name_w.end());
    }

    //将新节点塞入数组
    uint32_t new_index = (uint32_t)ctx.memory_tree.size();
    ctx.memory_tree.push_back(new_node);

    //递归克隆所有子孙节点
    uint32_t child_idx = ctx.memory_tree[source_idx].first_child;
    uint32_t prev_new_child_idx = INVALID_INDEX;

    while (child_idx != INVALID_INDEX) {
        //孩子不需要改名，名字传空
        uint32_t cloned_child_idx = update_memory_after_copy(ctx, child_idx, new_index, "");

        //尾插法串联新克隆的孩子们
        if (prev_new_child_idx == INVALID_INDEX) {
            ctx.memory_tree[new_index].first_child = cloned_child_idx;
        } else {
            ctx.memory_tree[prev_new_child_idx].next_sibling = cloned_child_idx;
        }

        prev_new_child_idx = cloned_child_idx;
        child_idx = ctx.memory_tree[child_idx].next_sibling;
    }

    return new_index;
}

bool general_control::execute_paste(const file_location& dest_folder) {
    if (m_clipboard_targets.isEmpty() || m_current_op == filemanager::clipboard_operation::None) {
        return false;
    }

    QString target_drive = dest_folder.drive;
    if (!drive_map.contains(target_drive)) return false;
    drive_content& ctx = drive_map[target_drive];

    bool all_success = true;

    for (const file_location& src_loc : std::as_const(m_clipboard_targets)) {
        //跨盘符暂不处理
        if (src_loc.drive != target_drive) {
            all_success = false;
            continue;
        }

        QString src_path = get_absolute_path(src_loc.drive, src_loc.index);
        QString src_name = get_node_name(src_loc.drive, src_loc.index);
        QString dest_dir_path = get_absolute_path(target_drive, dest_folder.index);

        //防止文件夹递归自己
        QString norm_src = QDir::cleanPath(src_path) + "/";
        QString norm_dest = QDir::cleanPath(dest_dir_path) + "/";
        if (norm_dest.startsWith(norm_src, Qt::CaseInsensitive)) {
            all_success = false;
            continue;
        }

        //拿到安全的新路径
        QString dest_path = file_worker.get_safe_destination_path(dest_dir_path, src_name);
        bool is_dir = ctx.memory_tree[src_loc.index].is_dir;

        //剪切
        if (m_current_op == filemanager::clipboard_operation::Cut) {
            if (file_worker.rename_file(src_path, dest_path)) {
                detach_node(ctx, src_loc.index);
                attach_node(ctx, src_loc.index, dest_folder.index);
            } else {
                all_success = false;
            }
        }
        //复制
        else if (m_current_op == filemanager::clipboard_operation::Copy) {
            bool phys_success = false;
            if (is_dir) {
                phys_success = file_worker.copy_directory(src_path, dest_path);
            } else {
                phys_success = QFile::copy(src_path, dest_path);
            }

            //内存同步
            if (phys_success) {
                QString new_safe_name = QFileInfo(dest_path).fileName();
                uint32_t cloned_idx = update_memory_after_copy(ctx, src_loc.index, dest_folder.index, new_safe_name);
                attach_node(ctx, cloned_idx, dest_folder.index);
            } else {
                all_success = false;
            }
        }
    }

    //只有剪切用完才清空剪贴板
    if (m_current_op == filemanager::clipboard_operation::Cut) {
        m_clipboard_targets.clear();
        m_current_op = filemanager::clipboard_operation::None;
    }

    emit paste_finished();
    return all_success;
}