#include "mainwindow.h"
#include "constants.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPushButton>
#include <QVBoxLayout>
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    // 文件岛

    m_dockIsland = new QDockWidget("文件岛", this);
    m_dockIsland->setAllowedAreas(Qt::BottomDockWidgetArea);
    m_dockIsland->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable );

    m_fileIsland = new fileIslandWidget(m_dockIsland);
    m_dockIsland->setWidget(m_fileIsland);
    m_dockIsland->setFixedHeight(180);
    addDockWidget(Qt::BottomDockWidgetArea, m_dockIsland);
    m_dockIsland->hide();
    connect(ui->fileislandBtn, &QPushButton::clicked, this, [this]() {
        if (m_dockIsland->isVisible()) {
            m_dockIsland->hide();
        } else {
            m_dockIsland->setFloating(false);
            m_dockIsland->show();
            m_dockIsland->raise();
        }
    });
    connect(m_dockIsland, &QDockWidget::topLevelChanged, this, [this](bool floating) {
        if (floating) {
            ui->fileislandBtn->setText("▼ 收起文件岛");
            m_dockIsland->setMaximumHeight(10000);
            m_dockIsland->setMinimumHeight(100);
        } else {
            m_dockIsland->setFixedHeight(180);
            ui->fileislandBtn->setText("▼ 收起文件岛");
        }
    });
    connect(m_dockIsland, &QDockWidget::visibilityChanged, this, [this](bool visible) {
        if (!visible || m_dockIsland->isFloating()) {
            ui->fileislandBtn->setText("▲ 呼出文件岛");
        } else {
            ui->fileislandBtn->setText("▼ 收起文件岛");
        }
    });

    // 设置 drivelist
    connect(ui->driveZone, &driveListZone::globalRefreshRequested, this, [this]() {
        qDebug() << "【主窗口】收到全局盘符刷新请求！正在获取系统硬盘...";
        QList<Helper::DriveInfo> drives = Helper::getAllDrives();
        ui->driveZone->updateDriveList(drives);
    });
    connect(ui->driveZone, &driveListZone::scanDriveRequested,
            this, [this](const QString &driveLetter, bool forceRefresh) {
                qDebug() << "【主窗口】收到扫描请求！盘符:" << driveLetter << " 是否强刷:" << forceRefresh;
                m_fileIsland->switchDrive(ui->breadcrumbline->getRootLetter());
                onScanStarted(driveLetter);
                m_generalControl->start_scan(driveLetter, forceRefresh);
            });
    emit ui->driveZone->globalRefreshRequested();
    // 设置 breadcrumb
    ui->breadcrumbline->setLabel("等待");
    // 设置 tableview 的表头



    // 总控类
    m_generalControl = new general_control(this);

    // 链接函数
    connect(ui->fileDisplayerWidget, &fileDisplayer::onFileDoubleClicked,
            this, &MainWindow::handleFileDoubleClicked);
    connect(ui->breadcrumbline, &breadcrumbWidget::pathClicked,
            this, &MainWindow::refreshTable);
    connect(m_generalControl, &general_control::scan_started,
            this, &MainWindow::onScanStarted);
    connect(m_generalControl, &general_control::scan_finished,
            this, &MainWindow::onScanFinished);
    connect(m_generalControl, &general_control::scan_error,
            this, &MainWindow::onScanError);
    connect(m_fileIsland, &fileIslandWidget::requestDelete, this, [=](const QList<file_location>& targets) {
        // 直接调后端
        bool success = m_generalControl->deleteFile(targets);
        if (success) {
            qDebug() << "删除成功，内存树已同步！";
        }
    });




}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::handleFileDoubleClicked(QString name, uint32_t index, bool isDir)
{
    qDebug() << "【主窗口】捕捉到双击信号！文件名:" << name << " 目录?:" << isDir;

    if (!isDir) {
        // 如果是文件，直接打开
        QString currentDir = ui->breadcrumbline->getAbsolutePath();
        QString filePath = QDir(currentDir).filePath(name);
        qDebug() << "准备调用系统打开路径：" << filePath;
        bool success = QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        if (!success) {
            qDebug() << "打开失败！";
        }
        return;
    }

    // 如果是文件夹，更新面包屑，并刷新表格
    qDebug() << "【双击下沉】准备进入文件夹:" << name << " 节点序号:" << index;
    ui->breadcrumbline->pushNode(name, index);
    refreshTable(index);
}



void MainWindow::refreshTable(uint32_t targetIndex){
    QString driveLetter = ui->breadcrumbline->getRootLetter();

    // 拿到后端的数据
    QList<UI_Block> fileDatas = m_generalControl->get_content(driveLetter, targetIndex);
    qDebug() << " 共找到:" << fileDatas.count() << "个子项";

    // 🌟 仅仅只需要这一行代码，所有的排序、更新、UI渲染全部自动搞定！
    ui->fileDisplayerWidget->setFiles(fileDatas);
    ui->fileDisplayerWidget->setCurrentPath(ui->breadcrumbline->getAbsolutePath());
}

void MainWindow::onScanStarted(const QString& drive_letter){
    // 第一步，清空原有数据
    ui->fileDisplayerWidget->setFiles(QList<UI_Block>());
    ui->breadcrumbline->setLabel("正在扫描" + drive_letter + "盘");
    ui->breadcrumbline->initRoot();
    // 第二步，锁死刷新和扫描按钮
    ui->driveZone->setEnabled(false);
    ui->breadcrumbline->setEnabled(false);


}

void MainWindow::onScanFinished(const QString& drive_letter, uint32_t root_index){
    qDebug() << "finished";
    // 第一步，清空原有数据
    ui->fileDisplayerWidget->setFiles(QList<UI_Block>());
    ui->breadcrumbline->setLabel("当前路径");
    ui->breadcrumbline->initRoot();
    ui->breadcrumbline->pushNode(drive_letter + "盘", root_index);
    // 第二步，恢复刷新和扫描按钮
    ui->driveZone->setEnabled(true);
    ui->breadcrumbline->setEnabled(true);
    // 第三步，循环添加信息
    refreshTable(root_index);

}

void MainWindow::onScanError(const QString& drive_letter, const QString& error_message){
    // 第一步，清空原有数据
    ui->fileDisplayerWidget->setFiles(QList<UI_Block>());
    // 第二步，恢复刷新和扫描按钮
    ui->driveZone->setEnabled(true);
    ui->breadcrumbline->setEnabled(true);
    // 第三步，报错
    qDebug() << error_message;
}








