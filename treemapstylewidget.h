#ifndef TREEMAPSTYLEWIDGET_H
#define TREEMAPSTYLEWIDGET_H

#include "datatype.h"
#include "treemapengine.h"

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

    // 设置大小压缩指数：1.0=线性 0.35=推荐甜点 0.0=等分
    void setExponent(double exp);
    double exponent() const { return m_exponent; }

signals:
    void itemDoubleClicked(uint32_t nodeIndex, bool isDir);
    void requestResizeUpdate(double width, double height, double exponent);

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
    double m_exponent = 0.5;
};

#endif // TREEMAPSTYLEWIDGET_H
