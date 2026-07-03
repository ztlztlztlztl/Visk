#ifndef FILEDISPLAYMODEL_H
#define FILEDISPLAYMODEL_H



#include "datatype.h"
#include "helper.h"


#include <QAbstractTableModel>
#include <QList>
#include <QMimeData>
#include <QDebug>

class fileDisplayModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum FileRoles {
        fileNameRole = Qt::UserRole + 1,
        fileByteSizeRole = Qt::UserRole + 2,
        fileTypeRole = Qt::UserRole + 3,
        FileTimeRole = Qt::UserRole + 4,
        fileIndexRole = Qt::UserRole + 5,
        fileIsDirRole = Qt::UserRole  +6,
    };

    explicit fileDisplayModel(QObject *parent = nullptr);

    void updateData(const QList<UI_Block>& newData);

    UI_Block getFileInfo(int row) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;

    void setCurrentBasePath(const QString &path);

private:
    QList<UI_Block> m_data;
    QString m_currentBasePath;
};













#endif // FILEDISPLAYMODEL_H
