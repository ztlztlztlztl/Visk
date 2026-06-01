#include "breadcrumbwidget.h"


breadcrumbWidget::breadcrumbWidget(QWidget *parent) : QWidget(parent) ,m_str(QString()){
    this->setStyleSheet(Constants::style_breadcrumbWidget_background);
    m_layout = new QHBoxLayout(this);          // 添加水平布局
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(4);
}

breadcrumbWidget::~breadcrumbWidget() {}

void breadcrumbWidget::rebuildUI() {
    // 1.清空
    QLayoutItem *child;
    while ((child = m_layout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->hide();
            child->widget()->setParent(nullptr);
            child->widget()->deleteLater();
        }
        delete child;
    }
    // 2. 重构
    QLabel *textLabel = new QLabel(m_str);
    textLabel->setStyleSheet(Constants::style_breadcrumbWidget_text);
    m_layout->addWidget(textLabel);
    for (int i = 0; i < m_pathStack.size(); ++i) {
        QString name = m_pathStack[i].first;
        uint32_t idx = m_pathStack[i].second;

        QPushButton *btn = new QPushButton(name, this);
        // 最后一层特殊处理
        if (i == m_pathStack.size() - 1) {
            btn->setStyleSheet(Constants::style_breadcrumbWidget_end);
            btn->setEnabled(false);
        } else {
            btn->setStyleSheet(Constants::style_breadcrumbWidget_normal);
        }

        // 点击面包屑某一层，截断数组并通知 MainWindow
        connect(btn, &QPushButton::clicked, this, [this, i, idx]() {
            m_pathStack.resize(i + 1);
            rebuildUI();
            emit pathClicked(idx);
        });
        m_layout->addWidget(btn);

        // 箭头
        if (i < m_pathStack.size() - 1) {
            QLabel *arrow = new QLabel(" > ", this);
            arrow->setStyleSheet(Constants::style_breadcrumbWidget_arrow);
            m_layout->addWidget(arrow);
        }
    }

    m_layout->addStretch(); // 弹簧
}


void breadcrumbWidget::initRoot() {
    m_pathStack.clear();
    rebuildUI();
}

void breadcrumbWidget::pushNode(const QString& folderName, uint32_t nodeIndex) {
    m_pathStack.append({folderName, nodeIndex});
    rebuildUI();
}

void breadcrumbWidget::setLabel(const QString& str){
    m_str = str;
    rebuildUI();
}

