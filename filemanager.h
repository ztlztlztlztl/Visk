#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QString>
#include <QDateTime>
#include <QList>
#include <QFileInfo>
#include <QStringList>

class filemanager
{
public:

    filemanager() = default;
    ~filemanager() = default;

    //剪切板操作类型
    enum class clipboard_operation {
        None,
        Copy,
        Cut
    };

    //重命名文件或文件夹
    bool rename_file(const QString& file_path, const QString& new_name);

    //删除文件或文件夹
    bool delete_file(const QString& file_path);

    //创建新文件夹
    bool create_new_directory(const QString& parent_path, const QString& directory_name);

    //递归拷贝文件夹
    bool copy_directory(const QString& source_directory, const QString& destination_directory);

    //处理同名文件
    QString get_safe_destination_path(const QString& file_path, const QString& file_name);
};

#endif //FILEMANAGER_H