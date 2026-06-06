#ifndef FILEDISPLAYWIDGET_H
#define FILEDISPLAYWIDGET_H
#include "constants.h"
#include "datatype.h"
#include "filedisplaymodel.h"
#include "tablestylewidget.h"
#include "treemapstylewidget.h"


#include <QWidget>
#include <QTabWidget>
#include <QList>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>



class fileSortProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit fileSortProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent), m_folderOnlyMode(false) {}

    void setFolderOnlyMode(bool folderOnly) {
        if (m_folderOnlyMode != folderOnly) {
            QSortFilterProxyModel::beginFilterChange();
            m_folderOnlyMode = folderOnly;
            QSortFilterProxyModel::endFilterChange();
        }
    }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
        if (m_folderOnlyMode) {
            QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
            bool isDir = sourceModel()->data(idx, fileDisplayModel::fileIsDirRole).toBool();
            if (!isDir) return false;
        }
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override {
        int col = left.column();
        if (col == 1) {
            qint64 leftSize = sourceModel()->data(left, fileDisplayModel::fileByteSizeRole).toLongLong();
            qint64 rightSize = sourceModel()->data(right, fileDisplayModel::fileByteSizeRole).toLongLong();
            return leftSize < rightSize;
        }
        if (col == 3) {
            QDateTime leftTime = sourceModel()->data(left, fileDisplayModel::FileTimeRole).toDateTime();
            QDateTime rightTime = sourceModel()->data(right, fileDisplayModel::FileTimeRole).toDateTime();
            return leftTime < rightTime;
        }
        return QSortFilterProxyModel::lessThan(left, right);
    }

private:
    bool m_folderOnlyMode; // 记忆当前模式
};





class fileDisplayer : public QWidget {
    Q_OBJECT
public:
    explicit fileDisplayer(QWidget *parent = nullptr);

    void setFiles(const QList<UI_Block>& files);

    void setCurrentPath(const QString &path);

    void setFolderOnlyMode(bool folderOnly);

    void setTreemapData(const std::vector<TreemapItem>& data, const QString& currentDrive);

signals:
    void onFileDoubleClicked(QString name, uint32_t index, bool isDir);

    void onTreemapDoubleClicked(uint32_t index, bool isDir);
    void requestTreemapUpdate(double w, double h, double exponent);

private slots:
    void onTableIndexDoubleClicked(const QModelIndex &index);

private:

    fileSortProxyModel* m_proxyModel;
    fileDisplayModel* m_fileModel;
    QTabWidget* m_tabWidget;
    tableStyleWidget* m_table;

    treemapStyleWidget* m_treemapWidget;
};






















#endif // FILEDISPLAYWIDGET_H
