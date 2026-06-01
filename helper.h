#ifndef HELPER_H
#define HELPER_H


#include <QString>
#include <QList>
#include <QLocale>
#include <QStorageInfo>

namespace Helper {
    struct DriveInfo{ // 记录一个磁盘的基本情况
        QString letter;
        QString name;
        QString filesystem;
        qint64 totalBytes;
        qint64 freeBytes;
        bool isNtfs;
    };

    QList<DriveInfo> getAllDrives();

    QString transToMemory(qint64 size);



}

#endif // HELPER_H
