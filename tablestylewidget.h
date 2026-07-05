#ifndef TABLESTYLEWIDGET_H
#define TABLESTYLEWIDGET_H

#include "constants.h"
#include "datatype.h"
#include "filedisplaymodel.h"

#include <QList>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QTableView>
#include <QHeaderView>
#include <QAbstractItemModel>
#include <QDrag>
#include <QMimeData>
#include <QApplication>

class tableStyleWidget: public QWidget {
    Q_OBJECT
public:
    explicit tableStyleWidget(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);

signals:
    void rowDoubleClicked(const QModelIndex &index);
    void rowContextMenuRequested(const QModelIndex &index, const QPoint &globalPos);

private:
    void setupTableAppearance();

    QTableView *m_tableView;
    QAbstractItemModel *m_model;
};





#endif // TABLESTYLEWIDGET_H
