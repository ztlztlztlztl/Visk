#include "tablestylewidget.h"







tableStyleWidget::tableStyleWidget(QWidget *parent)
    : QWidget(parent), m_model(nullptr)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_tableView = new QTableView(this);
    layout->addWidget(m_tableView);
    setupTableAppearance();
    connect(m_tableView, &QTableView::doubleClicked,
            this, &tableStyleWidget::rowDoubleClicked);
}

void tableStyleWidget::setupTableAppearance() {
    // 点击选中整行
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    // 禁用修改
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 隐藏行号
    m_tableView->verticalHeader()->setVisible(false);
    // 交替行颜色
    m_tableView->setAlternatingRowColors(true);
    // 取消网格线
    m_tableView->setShowGrid(false);

    QHeaderView *header = m_tableView->horizontalHeader();

    header->setStretchLastSection(true);


    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    m_tableView->setDragEnabled(true);
    m_tableView->setDragDropMode(QAbstractItemView::DragOnly);
    m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_tableView->setSortingEnabled(true);
}

void tableStyleWidget::setModel(QAbstractItemModel *model) {
    m_model = model;
    m_tableView->setModel(m_model);
}





