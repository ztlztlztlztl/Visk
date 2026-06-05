#include "helper.h"
#include <windows.h>

namespace Helper {
    QList<DriveInfo> getAllDrives(){
        QList<DriveInfo> allDrives;

        QList<QStorageInfo> storageList = QStorageInfo::mountedVolumes();

        for (const QStorageInfo &storage : storageList){
            if (!storage.isValid() || !storage.isReady() || storage.isReadOnly()) continue;
            DriveInfo info;
            info.letter = storage.rootPath().left(1).toUpper();
            info.name = storage.name().isEmpty() ? "本地磁盘" : storage.name();
            info.filesystem = QString::fromUtf8(storage.fileSystemType());
            info.totalBytes = storage.bytesTotal();
            info.freeBytes = storage.bytesFree();
            info.isNtfs = (info.filesystem.toUpper() == "NTFS");

            allDrives.append(info);
        }

        return allDrives;
    }

    QString transToMemory(qint64 size){
        const qint64 KB = 1024;
        const qint64 MB = KB * 1024;
        const qint64 GB = MB * 1024;
        const qint64 TB = GB * 1024;
        QString unit;
        double file_memory = static_cast<qint64>(size);
        if (size >= TB) {
            file_memory /= TB;
            unit = " TB";
        } else if (size >= GB) {
            file_memory /= GB;
            unit = " GB";
        } else if (size >= MB) {
            file_memory /= MB;
            unit = " MB";
        } else if (size >= KB) {
            file_memory /= KB;
            unit = " KB";
        } else {
            unit = " B";
        }
        return QLocale::system().toString(file_memory, 'f', 1) + unit;
    }

    QString getFileTypeString(const QString& fileName, bool isDirectory)
    {
        if (isDirectory) return QStringLiteral("文件夹");

        SHFILEINFOW sfi = { 0 };
        DWORD fileAttributes = FILE_ATTRIBUTE_NORMAL;
        SHGetFileInfoW(
            (LPCWSTR)fileName.utf16(),
            fileAttributes,
            &sfi,
            sizeof(sfi),
            SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES
            );
        QString typeStr = QString::fromWCharArray(sfi.szTypeName);
        if (typeStr.isEmpty()) {
            return QStringLiteral("文件");
        }
        return typeStr;
    }




}

