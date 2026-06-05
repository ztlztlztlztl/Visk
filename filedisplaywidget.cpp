#include "filedisplaywidget.h"






fileDisplayer::fileDisplayer(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_tabWidget = new QTabWidget(this);

    m_fileModel = new fileDisplayModel(this);


    // 🌟 1. 实例化并存入私有变量（注意加上 this，防止内存泄漏！）
    m_proxyModel = new fileSortProxyModel(this);
    m_proxyModel->setSourceModel(m_fileModel);

    m_table = new tableStyleWidget(this);
    m_table->setModel(m_proxyModel);

    m_tabWidget->addTab(m_table, "列表视图");

    // 🔮 6. 为未来的图形化（Treemap）小弟提前占个座
    // 等过两天写好了 TreemapWidget，代码直接长这样：
    // m_treemapWidget = new TreemapWidget(this);
    // m_treemapWidget->setModel(m_fileModel); // 共享同一个 Model！
    // m_tabWidget->addTab(m_treemapWidget, "图形化探索");
    m_tabWidget->addTab(new QWidget(this), "图形化探索"); // 暂用空页面占位

    // 把 Tab 放到布局里
    mainLayout->addWidget(m_tabWidget);

    connect(m_table, &tableStyleWidget::rowDoubleClicked,
            this, &fileDisplayer::onTableIndexDoubleClicked);

    // 等 Treemap 写好了，就把这行取消注释
    // connect(m_treemapWidget, &TreemapWidget::fileDoubleClicked,
    //         this, &ExplorerContentView::onChildDoubleClicked);


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