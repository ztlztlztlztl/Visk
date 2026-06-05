#ifndef BREADCRUMBWIDGET_H
#define BREADCRUMBWIDGET_H

#include "constants.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>


class breadcrumbWidget : public QWidget{
    Q_OBJECT

public:
    explicit breadcrumbWidget(QWidget *parent = nullptr);
    ~breadcrumbWidget();

    QList<QPair<QString, uint32_t>> m_pathStack;

    void rebuildUI();

    void initRoot();

    void pushNode(const QString& folderName, uint32_t nodeIndex);

    void setLabel(const QString& str);

    QString getAbsolutePath() const;

    QString getRootLetter() const;

signals:
    void pathClicked(uint32_t targetIndex);


private:
    QString m_str;

    QHBoxLayout *m_layout;

};
















#endif // BREADCRUMBWIDGET_H
