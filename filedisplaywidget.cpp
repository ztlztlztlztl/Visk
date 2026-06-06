#include "filedisplaywidget.h"






fileDisplayer::fileDisplayer(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_tabWidget = new QTabWidget(this);

    m_tabWidget->setStyleSheet(Constants::style_filedisplayer_bar);

    m_fileModel = new fileDisplayModel(this);


    // 列表
    m_proxyModel = new fileSortProxyModel(this);
    m_proxyModel->setSourceModel(m_fileModel);

    m_table = new tableStyleWidget(this);
    m_table->setModel(m_proxyModel);

    m_tabWidget->addTab(m_table, "列表视图");

    // 图形化
    m_treemapWidget = new treemapStyleWidget(this);
    m_tabWidget->addTab(m_treemapWidget, "图形化视图");

    mainLayout->addWidget(m_tabWidget);

    connect(m_table, &tableStyleWidget::rowDoubleClicked,
            this, &fileDisplayer::onTableIndexDoubleClicked);

    connect(m_treemapWidget, &treemapStyleWidget::itemDoubleClicked,
            this, &fileDisplayer::onTreemapDoubleClicked);

    connect(m_treemapWidget, &treemapStyleWidget::requestResizeUpdate,
            this, &fileDisplayer::requestTreemapUpdate);

    connect(m_tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 1) {
            emit requestTreemapUpdate(m_treemapWidget->width(), m_treemapWidget->height(), m_treemapWidget->exponent());
        }
    });

    // 搜索
    m_searchTab = new QWidget();
    QVBoxLayout* searchLayout = new QVBoxLayout(m_searchTab);
    searchLayout->setContentsMargins(4, 4, 4, 4);

    // 顶部搜索控制栏
    QHBoxLayout* searchTopLayout = new QHBoxLayout();
    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("输入文件名或后缀 (例如: .png, 报告)...");
    m_searchInput->setStyleSheet(Constants::style_filedisplayer_search_text);

    m_searchScopeCombo = new QComboBox();
    m_searchScopeCombo->addItem("当前目录");
    m_searchScopeCombo->addItem("当前磁盘全局");
    m_searchScopeCombo->setStyleSheet(Constants::style_filedisplayer_search_combo);
    m_searchScopeCombo->view()->parentWidget()->setStyleSheet("background-color: #FFFFFF;");
    m_searchScopeCombo->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    m_searchScopeCombo->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    m_searchBtn = new QPushButton("搜索");
    m_searchBtn->setStyleSheet(Constants::style_filedisplayer_search_button);

    searchTopLayout->addWidget(m_searchInput);
    searchTopLayout->addWidget(m_searchScopeCombo);
    searchTopLayout->addWidget(m_searchBtn);

    m_searchModel = new fileDisplayModel(this);
    m_searchProxyModel = new fileSortProxyModel(this);
    m_searchProxyModel->setSourceModel(m_searchModel);

    m_searchTable = new tableStyleWidget();
    m_searchTable->setModel(m_searchProxyModel);

    searchLayout->addLayout(searchTopLayout);
    searchLayout->addWidget(m_searchTable);

    m_tabWidget->addTab(m_searchTab, "搜索视图");

    // 把 Tab 放到布局里
    mainLayout->addWidget(m_tabWidget);

    connect(m_table, &tableStyleWidget::rowDoubleClicked,
            this, &fileDisplayer::onTableIndexDoubleClicked);

    connect(m_searchBtn, &QPushButton::clicked, this, &fileDisplayer::executeSearch);
    connect(m_searchInput, &QLineEdit::returnPressed, this, &fileDisplayer::executeSearch);

    connect(m_searchTable, &tableStyleWidget::rowDoubleClicked, this, &fileDisplayer::onSearchTableDoubleClicked);

}



void fileDisplayer::setFiles(const QList<UI_Block>& files) {
    if (m_fileModel) {
        m_fileModel->updateData(files);
    }
}

void fileDisplayer::onTableIndexDoubleClicked(const QModelIndex &index) {
    if (!index.isValid() || !m_fileModel || !m_proxyModel) return;

    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);

    UI_Block clickedBlock = m_fileModel->getFileInfo(sourceIndex.row());

    emit onFileDoubleClicked(clickedBlock.file_name, clickedBlock.file_index, clickedBlock.is_directory);
}

void fileDisplayer::setCurrentPath(const QString &path) {
    if (m_fileModel) {
        m_fileModel->setCurrentBasePath(path);
    }
    if (m_searchModel) {
        m_searchModel->updateData(QList<UI_Block>());
    }
}

void fileDisplayer::setFolderOnlyMode(bool folderOnly) {
    if (m_proxyModel) {
        m_proxyModel->setFolderOnlyMode(folderOnly);
    }
}

void fileDisplayer::setTreemapData(const std::vector<TreemapItem>& data, const QString& currentDrive) {
    if (m_treemapWidget) {
        m_treemapWidget->setTreemapData(data, currentDrive);
    }
}

void fileDisplayer::executeSearch() {
    QString keyword = m_searchInput->text().trimmed();
    if (keyword.isEmpty()) return;

    bool isGlobal = (m_searchScopeCombo->currentIndex() == 1);

    emit requestSearch(keyword, isGlobal);
}

void fileDisplayer::setSearchResults(const QList<UI_Block>& results) {
    m_searchModel->updateData(results);
}

void fileDisplayer::onSearchTableDoubleClicked(const QModelIndex &index) {
    if (!index.isValid()) return;

    QModelIndex sourceIndex = m_searchProxyModel->mapToSource(index);
    UI_Block info = m_searchModel->getFileInfo(sourceIndex.row());
    if (info.is_directory) {
        m_tabWidget->setCurrentIndex(0); // 0 就是最左边的“列表视图”
    }
    emit onFileDoubleClicked(info.file_name, info.file_index, info.is_directory);
}