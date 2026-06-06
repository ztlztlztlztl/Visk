#include "treemapstylewidget.h"


treemapStyleWidget::treemapStyleWidget(QWidget *parent) : QWidget(parent) {
    this->setMouseTracking(true);
}

void treemapStyleWidget::setTreemapData(const std::vector<TreemapItem>& data, const QString& currentDrive) {
    m_data = data;
    m_currentDrive = currentDrive;
    this->update();
}

void treemapStyleWidget::setExponent(double exp) {
    if (m_exponent != exp) {
        m_exponent = exp;
        // 指数变了 → 触发重算
        emit requestResizeUpdate(this->width(), this->height(), m_exponent);
    }
}

void treemapStyleWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(this->rect(), QColor(240, 240, 240));

    for (const TreemapItem& item : m_data) {
        QRectF rect(item.x, item.y, item.w, item.h);

        if (item.is_directory) {
            painter.setBrush(QColor(173, 216, 230)); // 文件夹偏蓝
        } else {
            painter.setBrush(QColor(169, 216, 183)); // 文件偏灰
        }
        painter.setPen(QPen(Qt::white, 1)); // 白边框
        painter.drawRect(rect);

        // 画文字（如果方块太小就算了，免得字挤在一起）
        if (item.w > 40 && item.h > 20) {
            painter.setPen(Qt::black);
            painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, item.name);
        }
    }
}

void treemapStyleWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
    }
}

void treemapStyleWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!(event->buttons() & Qt::LeftButton)) return;
    if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) return;

    const TreemapItem* clickedItem = itemAt(m_dragStartPosition);
    if (!clickedItem) return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData();
    QString payload = QString("%1|%2").arg(m_currentDrive).arg(clickedItem->node_index);
    mimeData->setText(payload);
    drag->setMimeData(mimeData);

    drag->exec(Qt::CopyAction | Qt::MoveAction);
}

void treemapStyleWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        const TreemapItem* clickedItem = itemAt(event->pos());
        if (clickedItem) {
            emit itemDoubleClicked(clickedItem->node_index, clickedItem->is_directory);
        }
    }
}

void treemapStyleWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    emit requestResizeUpdate(this->width(), this->height(), m_exponent);
}

const TreemapItem* treemapStyleWidget::itemAt(const QPoint& pos) const {
    for (const TreemapItem& item : m_data) {
        QRectF rect(item.x, item.y, item.w, item.h);
        if (rect.contains(pos)) {
            return &item;
        }
    }
    return nullptr;
}