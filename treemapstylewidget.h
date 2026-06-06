#ifndef TREEMAPSTYLEWIDGET_H
#define TREEMAPSTYLEWIDGET_H

#include "datatype.h"
#include "treemapengine.h"
#include "helper.h"

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <vector>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QDebug>

class treemapStyleWidget : public QWidget {
    Q_OBJECT
public:
    explicit treemapStyleWidget(QWidget *parent = nullptr);

    void setTreemapData(const std::vector<TreemapItem>& data, const QString& currentDrive);

signals:
    void itemDoubleClicked(uint32_t nodeIndex, bool isDir);
    void requestResizeUpdate(double width, double height);

protected:
    // 画图
    void paintEvent(QPaintEvent *event) override;
    // 交互与拖拽
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    // 尺寸改变核心
    void resizeEvent(QResizeEvent *event) override;

private:
    const TreemapItem* itemAt(const QPoint& pos) const;

    std::vector<TreemapItem> m_data;
    QString m_currentDrive;
    QPoint m_dragStartPosition;
};

#endif // TREEMAPSTYLEWIDGET_H
