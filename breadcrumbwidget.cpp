#include "breadcrumbwidget.h"

breadcrumbWidget::breadcrumbWidget(QWidget *parent) : QWidget(parent), m_str(QString()) {
    this->setStyleSheet(Constants::style_breadcrumbWidget_background);
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(4);
}

breadcrumbWidget::~breadcrumbWidget() {}

void breadcrumbWidget::setPath(const QList<QPair<QString, file_location>>& pathNodes) {
    m_pathNodes = pathNodes;
    rebuildUI();
}

void breadcrumbWidget::setLabel(const QString& str) {
    m_str = str;
    rebuildUI();
}

void breadcrumbWidget::rebuildUI() {
    QLayoutItem *child;
    while ((child = m_layout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->hide();
            child->widget()->deleteLater();
        }
        delete child;
    }

    QLabel *textLabel = new QLabel(m_str);
    textLabel->setStyleSheet(Constants::style_breadcrumbWidget_text);
    m_layout->addWidget(textLabel);

    for (int i = 0; i < m_pathNodes.size(); ++i) {
        QString name = m_pathNodes[i].first;
        file_location loc = m_pathNodes[i].second; // 这层的万能钥匙

        QPushButton *btn = new QPushButton(name, this);

        // 最后一层特殊处理：当前目录置灰不可点击
        if (i == m_pathNodes.size() - 1) {
            btn->setStyleSheet(Constants::style_breadcrumbWidget_end);
            btn->setEnabled(false);
        } else {
            btn->setStyleSheet(Constants::style_breadcrumbWidget_normal);
        }

        connect(btn, &QPushButton::clicked, this, [this, loc]() {
            emit pathClicked(loc);
        });

        m_layout->addWidget(btn);

        if (i < m_pathNodes.size() - 1) {
            QLabel *arrow = new QLabel(" > ", this);
            arrow->setStyleSheet(Constants::style_breadcrumbWidget_arrow);
            m_layout->addWidget(arrow);
        }
    }

    m_layout->addStretch();
}