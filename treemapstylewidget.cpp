#include "treemapstylewidget.h"


treemapStyleWidget::treemapStyleWidget(QWidget *parent) : QWidget(parent) {
    this->setMouseTracking(true);
}

void treemapStyleWidget::setTreemapData(const std::vector<TreemapItem>& data, const QString& currentDrive) {
    m_data = data;
    m_currentDrive = currentDrive;
    this->update();
}

void treemapStyleWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(248, 249, 250));

    for (const TreemapItem& item : m_data) {
        QRectF rect(item.x + 1, item.y + 1, item.w - 2, item.h - 2);

        if (item.is_directory) {
            painter.setBrush(QColor(192, 216, 243));
        } else {
            painter.setBrush(QColor(214, 235, 222));
        }

        painter.setPen(Qt::NoPen);

        painter.drawRect(rect);

        if (item.w > 45 && item.h > 35) {
            painter.setPen(QColor("#4A5568"));
            QString sizeStr = Helper::transToMemory(item.size);
            QString displayText = QString("%1\n%2").arg(item.name, sizeStr);

            QRectF textRect = rect.adjusted(4, 4, -4, -4);
            painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, displayText);
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
    emit requestResizeUpdate(this->width(), this->height());
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