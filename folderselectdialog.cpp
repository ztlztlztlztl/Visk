#include "folderselectdialog.h"

folderSelectDialog::folderSelectDialog(general_control* gc, file_location startLoc, QWidget* parent)
    : QDialog(parent), m_gc(gc)
{
    this->setWindowTitle("请选择目标文件夹");
    this->resize(600, 450);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    m_breadcrumb = new breadcrumbWidget(this);
    m_displayer = new fileDisplayer(this);
    m_displayer->setFolderOnlyMode(true);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* btnOk = new QPushButton("确定选择此文件夹", this);
    QPushButton* btnCancel = new QPushButton("取消", this);

    btnOk->setStyleSheet(Constants::style_fileselectdialog_check_button);

    btnLayout->addStretch();
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);

    mainLayout->addWidget(m_breadcrumb);
    mainLayout->addWidget(m_displayer, 1);
    mainLayout->addLayout(btnLayout);

    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);

    connect(m_breadcrumb, &breadcrumbWidget::pathClicked, this, &folderSelectDialog::navigateTo);
    connect(m_displayer, &fileDisplayer::onFileDoubleClicked, this, [this](QString name, uint32_t idx, bool isDir) {
        if (isDir) {
            navigateTo(file_location{m_currentLoc.drive, idx});
        }
    }, Qt::QueuedConnection);

    navigateTo(startLoc);
}

void folderSelectDialog::navigateTo(const file_location& loc) {
    if (loc.index == INVALID_INDEX) return;
    m_currentLoc = loc;

    QList<UI_Block> fileDatas = m_gc->get_content(loc.drive, loc.index);
    m_displayer->setFiles(fileDatas);

    QList<file_location> rawChain = m_gc->get_path_chain(loc);
    QList<QPair<QString, file_location>> breadcrumbData;
    for (const file_location& stepLoc : rawChain) {
        QString folderName = m_gc->get_node_name(stepLoc.drive, stepLoc.index);
        if (folderName.isEmpty()) folderName = stepLoc.drive + ":\\";
        breadcrumbData.append(qMakePair(folderName, stepLoc));
    }
    m_breadcrumb->setPath(breadcrumbData);
}
