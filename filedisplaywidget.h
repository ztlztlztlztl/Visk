#ifndef FILEDISPLAYWIDGET_H
#define FILEDISPLAYWIDGET_H
#include "constants.h"
#include "datatype.h"
#include "filedisplaymodel.h"
#include "tablestylewidget.h"


#include <QWidget>
#include <QTabWidget>
#include <QList>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>



class fileSortProxyModel : public QSortFilterProxyModel {
public:
    explicit fileSortProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent) {}
protected:
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
};





class fileDisplayer : public QWidget {
    Q_OBJECT
public:
    explicit fileDisplayer(QWidget *parent = nullptr);

    void setFiles(const QList<UI_Block>& files);

    void setCurrentPath(const QString &path);

signals:
    void onFileDoubleClicked(QString name, uint32_t index, bool isDir);

private slots:
    void onTableIndexDoubleClicked(const QModelIndex &index);

private:


    QSortFilterProxyModel* m_proxyModel;
    fileDisplayModel* m_fileModel;
    QTabWidget* m_tabWidget;
    tableStyleWidget* m_table;
};






















#endif // FILEDISPLAYWIDGET_H
