#include "filemanager.h"
#include <QDir>
#include <QFile>
#include <QDebug>

bool filemanager::rename_file(const QString& file_path, const QString& new_name) {
    QFileInfo info(file_path);
    if (!info.exists()) {
        return false;
    }

    QDir parent_dir = info.dir();
    QString new_path = parent_dir.absoluteFilePath(new_name);

    //防止命名冲突
    if (QFile::exists(new_path)) return false;

    return QFile::rename(file_path, new_path);
}

bool filemanager::delete_file(const QString& file_path) {
    QFileInfo info(file_path);
    if (!info.exists()) return false;

    if (info.isDir()) {
        QDir dir(file_path);
        return dir.removeRecursively();
    }
    else {
        return QFile::remove(file_path);
    }
}

bool filemanager::create_new_directory(const QString& parent_path, const QString& directory_name) {
    QDir dir(parent_path);
    if (!dir.exists()) return false;

    return dir.mkdir(directory_name);
}

bool filemanager::copy_directory(const QString& source_directory, const QString& destination_directory) {
    QDir source(source_directory);
    if (!source.exists()) {
        return false;
    }

    QDir dest(destination_directory);
    if (!dest.exists()) {
        dest.mkpath(".");
    }

    QFileInfoList info_list = source.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (const QFileInfo& info : std::as_const(info_list)) {
        QString dest_path = destination_directory + "/" + info.fileName();

        if (info.isDir()) {
            //递归调用
            if (!copy_directory(info.absoluteFilePath(), dest_path)) {
                return false;
            }
        }
        else {
            if (!QFile::copy(info.absoluteFilePath(), dest_path)) {
                return false;
            }
        }
    }
    return true;
}

QString filemanager::get_safe_destination_path(const QString& file_path, const QString& file_name) {
    QDir dir(file_path);
    QString base_name = file_name;
    QString suffix = "";

    QFileInfo info(dir.absoluteFilePath(file_name));
    //拆分后缀
    if (info.isFile() && file_name.contains(('.'))) {
        base_name = info.completeBaseName();
        suffix = "." + info.suffix();
    }

    QString new_filename = file_name;
    QString target_path = dir.absoluteFilePath(new_filename);

    int counter = 1;

    while (QFile::exists(target_path)) {
        new_filename = QString("%1(%2)%3").arg(base_name).arg(counter).arg(suffix);
        target_path = dir.absoluteFilePath(new_filename);
        counter++;
    }

    return target_path;
}