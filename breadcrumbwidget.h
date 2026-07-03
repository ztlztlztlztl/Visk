#ifndef BREADCRUMBWIDGET_H
#define BREADCRUMBWIDGET_H

#include "constants.h"
#include "datatype.h"
#include "general_control.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QList>
#include <QPair>

class breadcrumbWidget : public QWidget {
    Q_OBJECT

public:
    explicit breadcrumbWidget(QWidget *parent = nullptr);
    ~breadcrumbWidget();

    void setPath(const QList<QPair<QString, file_location>>& pathNodes);

    void setLabel(const QString& str);

signals:
    void pathClicked(const file_location& targetLoc);

private:
    void rebuildUI();

    QString m_str;
    QHBoxLayout *m_layout;

    QList<QPair<QString, file_location>> m_pathNodes;
};

#endif // BREADCRUMBWIDGET_H