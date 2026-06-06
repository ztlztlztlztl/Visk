#include "fileislandwidget.h"


fileIslandWidget::fileIslandWidget(QWidget *parent)
    : QWidget(parent)
{
    this->setStyleSheet(Constants::style_fileislandWidget_background);

    this->setAcceptDrops(true);
    setupUI();
    m_listWidget->setAcceptDrops(false);

    checkReadyState();
}

void fileIslandWidget::setupUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 5);
    mainLayout->setSpacing(8);

    // Object
    m_listWidget = new QListWidget(this);
    m_listWidget->setAlternatingRowColors(true);
    m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    m_listWidget->setStyleSheet(Constants::style_fileislandWidget_object);
    mainLayout->addWidget(m_listWidget, 1);

    // Action
    QVBoxLayout *actionLayout = new QVBoxLayout();
    actionLayout->setSpacing(5);
    actionLayout->setAlignment(Qt::AlignTop);
    m_actionGroup = new QButtonGroup(this);
    m_actionGroup->setExclusive(true);
    m_btnCopy   = new QPushButton("复制", this);
    m_btnMove   = new QPushButton("移动", this);
    m_btnDelete = new QPushButton("删除", this);
    m_btnRename = new QPushButton("改后缀", this);
    m_btnSystemCopy = new QPushButton("复制到剪贴板", this);
    QPushButton* actionBtns[] = {m_btnCopy, m_btnMove, m_btnDelete, m_btnRename, m_btnSystemCopy};
    int actionIds[] = {1, 2, 3, 4, 5}; // 给每个动作编个号 (0留给空闲状态)

    for (int i = 0; i < 5; ++i) {
        actionBtns[i]->setCheckable(true);
        actionBtns[i]->setFixedWidth(100);
        actionBtns[i]->setStyleSheet(Constants::style_fileislandWidget_action_button);
        m_actionGroup->addButton(actionBtns[i], actionIds[i]);
        actionLayout->addWidget(actionBtns[i]);
    }
    actionLayout->addStretch();
    mainLayout->addLayout(actionLayout);

    // Destination
    QVBoxLayout *rightLayout = new QVBoxLayout();

    m_destinationStack = new QStackedWidget(this);
    m_destinationStack->setFrameShape(QFrame::NoFrame);
    m_destinationStack->setLineWidth(0);
    m_destinationStack->setStyleSheet(Constants::style_fileislandWidget_destination_stack);

    // 0
    m_pageEmpty = new QWidget();
    QVBoxLayout *emptyLayout = new QVBoxLayout(m_pageEmpty);
    QLabel *emptyLabel = new QLabel("请在左侧选择要执行的操作");
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setStyleSheet(Constants::style_fileislandWidget_destination_text);
    emptyLayout->addWidget(emptyLabel);
    m_destinationStack->addWidget(m_pageEmpty); // Index 0

    // 1
    m_pagePath = new QWidget();
    QVBoxLayout *pathLayout = new QVBoxLayout(m_pagePath);
    QLabel *pathLabel = new QLabel("点击 do 按钮以选择目标文件夹", m_pagePath);
    pathLabel->setAlignment(Qt::AlignCenter);
    pathLabel->setStyleSheet(Constants::style_fileislandWidget_destination_text); // 极客蓝提示
    pathLayout->addWidget(pathLabel);
    m_destinationStack->addWidget(m_pagePath);  // Index 1

    // 2
    m_pageDelete = new QWidget();
    QVBoxLayout *deleteLayout = new QVBoxLayout(m_pageDelete);
    m_deleteWarning = new QLabel("警告：岛内所有对象将被直接移至系统回收站", m_pageDelete);
    m_deleteWarning->setStyleSheet(Constants::style_fileislandWidget_destination_delete_warning); // 警示红
    m_deleteWarning->setAlignment(Qt::AlignCenter);
    deleteLayout->addWidget(m_deleteWarning);
    m_destinationStack->addWidget(m_pageDelete); // Index 2

    // 3
    m_pageRename = new QWidget();
    QHBoxLayout *renameLayout = new QHBoxLayout(m_pageRename);
    renameLayout->setContentsMargins(16, 0, 16, 0);
    renameLayout->setSpacing(8);
    QLabel *renameLabel = new QLabel("新后缀名:", m_pageRename);
    m_extInput = new QLineEdit(m_pageRename);
    m_extInput->setPlaceholderText("例如: .png, .txt");
    renameLayout->addWidget(renameLabel);
    renameLayout->addWidget(m_extInput);
    m_pageRename->setStyleSheet(Constants::style_fileislandWidget_rename_page);
    m_destinationStack->addWidget(m_pageRename); // Index 3

    // 4
    m_pageSystemCopy = new QWidget();
    QVBoxLayout *sysCopyLayout = new QVBoxLayout(m_pageSystemCopy);
    QLabel *sysCopyLabel = new QLabel("点击 do 将文件放入系统剪贴板\n随后可在微信、QQ或桌面直接 Ctrl+V 粘贴", m_pageSystemCopy);
    sysCopyLabel->setAlignment(Qt::AlignCenter);
    sysCopyLabel->setStyleSheet(Constants::style_fileislandWidget_destination_text); // 极客蓝提示
    sysCopyLayout->addWidget(sysCopyLabel);
    m_destinationStack->addWidget(m_pageSystemCopy); // Index 4

    // Do
    m_btnDo = new QPushButton("do", this);
    m_btnDo->setFixedSize(80, 32);
    // 样式：极客绿
    m_btnDo->setStyleSheet(Constants::style_fileislandWidget_destination_do_button);
    QHBoxLayout *doLayout = new QHBoxLayout();
    doLayout->addStretch();
    doLayout->addWidget(m_btnDo);

    rightLayout->addWidget(m_destinationStack, 1);
    rightLayout->addLayout(doLayout);
    mainLayout->addLayout(rightLayout, 1);

    connect(m_actionGroup, &QButtonGroup::idClicked, this, &fileIslandWidget::onActionToggled);
    connect(m_btnDo, &QPushButton::clicked, this, &fileIslandWidget::onDoButtonClicked);
    connect(m_extInput, &QLineEdit::textChanged, this, &fileIslandWidget::checkReadyState);
}

void fileIslandWidget::onActionToggled(int id) {
    qDebug() << "选中的动作 ID:" << id;

    if (id == 1 || id == 2) {
        m_destinationStack->setCurrentIndex(1); // 复制和移动都跳到路径输入页
    } else if (id == 3) {
        m_destinationStack->setCurrentIndex(2); // 预留的删除页
    } else if (id == 4) {
        m_destinationStack->setCurrentIndex(3); // 预留的改后缀页
    }else if (id == 5) {
        m_destinationStack->setCurrentIndex(4); // 剪贴板提示页
    }

    checkReadyState();
}

void fileIslandWidget::checkReadyState() {
    bool hasFiles = m_listWidget->count() > 0;
    int actionId = m_actionGroup->checkedId();

    bool isParamReady = false;

    if (actionId == 1 || actionId == 2 || actionId == 3 || actionId == 5) {
        isParamReady = true;
    } else if (actionId == 4) {
        isParamReady = !m_extInput->text().trimmed().isEmpty();
    }
    m_btnDo->setEnabled(hasFiles && (actionId != -1) && isParamReady);
}

void fileIslandWidget::onDoButtonClicked() {
    int actionId = m_actionGroup->checkedId();
    if (actionId == 0) return;

    QList<file_node> currentFiles = m_islandData[m_currentDrive];
    if (currentFiles.isEmpty()) return;

    QList<file_location> targets;
    for (const file_node& node : currentFiles) {
        targets.append(node.fl);
    }
    switch (actionId) {
    case 1:
        qDebug() << "请求复制";
        emit requestCopyMoveDialog(targets, false);
        break;

    case 2:
        qDebug() << "请求移动";
        emit requestCopyMoveDialog(targets, true);
        break;

    case 3:
        qDebug() << "请求批量删除" << targets.size() << "个文件";
        qDebug() << targets[0].drive;
        emit requestDelete(targets);
        break;

    case 4:{
        QString newExt = m_extInput->text();
        if (!newExt.startsWith(".")) newExt.prepend(".");
        qDebug() << "请求批量改后缀为:" << newExt;
        emit requestRenameExt(targets, newExt);
        break;
    }

    case 5:
        qDebug() << "请求复制到系统剪贴板";
        emit requestSystemCopy(targets);
        break;
    }
    m_islandData[m_currentDrive].clear();
    refreshUI();
}

void fileIslandWidget::onRemoveItemButtonClicked() {
    QPushButton *clickedButton = qobject_cast<QPushButton*>(sender());
    if (!clickedButton) return;
    bool ok;
    int rowIndex = clickedButton->property("row_index").toInt(&ok);
    if (ok && rowIndex >= 0 && rowIndex < m_islandData[m_currentDrive].count()) {
        qDebug() << "【减号移出】正在将第" << rowIndex << "行文件剔除内存账本";
        m_islandData[m_currentDrive].removeAt(rowIndex);
        refreshUI();
    }
}


void fileIslandWidget::showIsland() {
    this->show();
}

void fileIslandWidget::hideIsland() {
    this->hide();
}

void fileIslandWidget::addFileToCurrentIsland(QString name, uint32_t index, QString absolutePath) {
    for (const file_node &node : m_islandData[m_currentDrive]) {
        if (node.fl.index == index) {
            qDebug() << "【拦截重复】文件已在岛内，拒绝重复添加！文件名：" << name;
            return;
        }
    }
    file_location newdata = {m_currentDrive, index};
    m_islandData[m_currentDrive].append(file_node{newdata, name, absolutePath});
    refreshUI();
}

void fileIslandWidget::switchDrive(const QString &driveLetter) {
    if (driveLetter.isEmpty()) return;
    QString cleanLetter = driveLetter.left(1).toUpper();
    m_currentDrive = cleanLetter;
    if (!m_islandData.contains(cleanLetter)) {
        m_islandData[cleanLetter] = QList<file_node>();
    }
    refreshUI();
}

void fileIslandWidget::refreshUI() {
    m_listWidget->clear();
    QList<file_node> currentFiles = m_islandData[m_currentDrive];

    for (int i = 0; i < currentFiles.count(); ++i) {
        const file_node &file = currentFiles[i];
        QListWidgetItem *item = new QListWidgetItem(m_listWidget);
        item->setSizeHint(QSize(0, 36));
        QWidget *rowContainer = new QWidget(m_listWidget);
        QHBoxLayout *rowLayout = new QHBoxLayout(rowContainer);
        rowLayout->setContentsMargins(8, 0, 8, 0);
        rowLayout->setSpacing(6);
        QString displayText = QString("%1 (%2)").arg(file.filename, file.fileabsolutepath);
        QLabel *textLabel = new QLabel(displayText, rowContainer);
        rowLayout->addWidget(textLabel, 1);
        QPushButton *btnRemove = new QPushButton("-", rowContainer);
        btnRemove->setFixedSize(24, 24);
        btnRemove->setObjectName("btnMinus");
        btnRemove->setCursor(Qt::PointingHandCursor);
        btnRemove->setProperty("row_index", i);
        rowLayout->addWidget(btnRemove);
        connect(btnRemove, &QPushButton::clicked, this, &fileIslandWidget::onRemoveItemButtonClicked);
        m_listWidget->setItemWidget(item, rowContainer);
    }
    checkReadyState();
}


void fileIslandWidget::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void fileIslandWidget::dropEvent(QDropEvent *event) {
    QString dropText = event->mimeData()->text();
    QStringList lines = dropText.split('\n', Qt::SkipEmptyParts);
    QList<file_location> droppedLocs;

    for (const QString &line : lines) {
        QStringList parts = line.split('|');
        if (parts.size() == 2) {
            file_location loc = {parts[0], parts[1].toUInt()};
            droppedLocs.append(loc);
        }
    }

    if (!droppedLocs.isEmpty()) {
        emit filesDropped(droppedLocs);
    }

    event->acceptProposedAction();
}