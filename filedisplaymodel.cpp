#include "filedisplaymodel.h"


fileDisplayModel::fileDisplayModel(QObject *parent): QAbstractTableModel(parent) {
}

void fileDisplayModel::updateData(const QList<UI_Block>& newData) {
    beginResetModel();
    m_data = newData;
    endResetModel();
}

UI_Block fileDisplayModel::getFileInfo(int row) const {
    if (row < 0 || row >= m_data.size()) return UI_Block();
    return m_data.at(row);
}

int fileDisplayModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0; // 扁平表格，不支持树状展开
    return m_data.size();
}

int fileDisplayModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return 4;
}

QVariant fileDisplayModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    const UI_Block& file = m_data.at(index.row());

    if (role == Qt::TextAlignmentRole) {
        int col = index.column();
        if (col == 1) {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        }
        if (col == 3) {
            return int(Qt::AlignCenter);
        }
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    }
    else if (role == Qt::DisplayRole) {
        int col = index.column();
        switch (col) {
        case 0: return file.file_name;
        case 1: return Helper::transToMemory(file.size);
        case 2: return Helper::getFileTypeString(file.file_name, file.is_directory);
        case 3: return file.last_modified.toString("yyyy-MM-dd hh:mm:ss");
        }
    }
    else if (role == fileNameRole) {
        return file.file_name;
    }
    else if (role == fileIndexRole) {
        return file.file_index;
    }
    else if (role == fileByteSizeRole) {
        return file.size;
    }
    else if (role == fileTypeRole) {
        return Helper::getFileTypeString(file.file_name, file.is_directory);
    }
    else if (role == FileTimeRole){
        return file.last_modified;
    }
    else if (role == fileIsDirRole){
        return file.is_directory;
    }

    return QVariant();
}

QVariant fileDisplayModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return "名称";
        case 1: return "大小";
        case 2: return "类型";
        case 3: return "修改时间";
        }
    }
    return QVariant();
}

Qt::ItemFlags fileDisplayModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);
    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | defaultFlags;
    }
    return defaultFlags;
}

QStringList fileDisplayModel::mimeTypes() const {
    return QStringList() << "text/plain";
}

QMimeData *fileDisplayModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QStringList payloadList;
    for (const QModelIndex &index : indexes) {
        if (index.column() == 0) {
            uint32_t fileIndex = data(index, fileIndexRole).toUInt();
            QString fileName = data(index, fileNameRole).toString();
            QString absolutePath = QDir(m_currentBasePath).filePath(fileName);
            QString payload = QString("%1|%2|%3").arg(fileIndex).arg(fileName, absolutePath);
            payloadList.append(payload);
        }
    }
    mimeData->setText(payloadList.join('\n'));
    return mimeData;
}


void fileDisplayModel::setCurrentBasePath(const QString &path) {
    m_currentBasePath = path;
}
