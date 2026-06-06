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
            emit requestTreemapUpdate(m_treemapWidget->width(), m_treemapWidget->height());
        }
    });

    // 把 Tab 放到布局里
    mainLayout->addWidget(m_tabWidget);

    connect(m_table, &tableStyleWidget::rowDoubleClicked,
            this, &fileDisplayer::onTableIndexDoubleClicked);



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