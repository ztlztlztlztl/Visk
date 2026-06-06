#include "mainwindow.h"
#include "constants.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPushButton>
#include <QVBoxLayout>
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), m_currentFileLocation(file_location())
{
    ui->setupUi(this);


    // 文件岛
    ui->fileislandBtn->setCheckable(true);
    ui->fileislandBtn->setStyleSheet(Constants::style_fileisland_call_button);

    m_dockIsland = new QDockWidget("", this);
    m_dockIsland->setAllowedAreas(Qt::BottomDockWidgetArea);
    m_dockIsland->setFeatures(QDockWidget::DockWidgetMovable);
    m_dockIsland->setTitleBarWidget(new QWidget(m_dockIsland));

    m_fileIsland = new fileIslandWidget(m_dockIsland);
    m_dockIsland->setWidget(m_fileIsland);
    m_dockIsland->setFixedHeight(220);
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
                m_fileIsland->switchDrive(driveLetter);
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
            this, &MainWindow::handleFileDoubleClicked, Qt::QueuedConnection);
    connect(ui->breadcrumbline, &breadcrumbWidget::pathClicked,
            this, &MainWindow::navigateTo);
    connect(m_generalControl, &general_control::scan_started,
            this, &MainWindow::onScanStarted);
    connect(m_generalControl, &general_control::scan_finished,
            this, &MainWindow::onScanFinished);
    connect(m_generalControl, &general_control::scan_error,
            this, &MainWindow::onScanError);
    connect(m_fileIsland, &fileIslandWidget::filesDropped, this, [=](const QList<file_location>& locs) {
        for (const file_location& loc : locs) {
            UI_Block info = m_generalControl->get_target_content(loc.drive, loc.index);
            if (info.file_index != INVALID_INDEX) {
                m_fileIsland->addFileToCurrentIsland(info.file_name, info.file_index, m_generalControl->get_absolute_path(loc.drive, loc.index));
            }
        }
    });
    connect(m_fileIsland, &fileIslandWidget::requestDelete, this, [=](const QList<file_location>& targets) {
        // 直接调后端
        bool success = m_generalControl->deleteFile(targets);
        if (!success) {
            return;
        }
        QList<file_location> chain = m_generalControl->get_path_chain(m_currentFileLocation);
        bool isCurrentFolderAlive = false;
        if (!chain.isEmpty()) {
            file_location rootLoc = chain.first();
            QString rootPath = m_generalControl->get_absolute_path(rootLoc.drive, rootLoc.index);
            if (rootPath == rootLoc.drive + ":\\") {
                isCurrentFolderAlive = true;
            }
        }
        if (isCurrentFolderAlive) {
            navigateTo(m_currentFileLocation);
        }
        else {
            QString currentDrive = m_currentFileLocation.drive.toUpper();
            uint32_t rootIdx = m_driveRoots.value(currentDrive, INVALID_INDEX);

            if (rootIdx != INVALID_INDEX) {
                qDebug() << "⚠️【避险警报】当前浏览目录已被无情删除！正在紧急撤离至" << currentDrive << "盘根目录...";
                navigateTo(file_location{currentDrive, rootIdx});
            }
        }
    });

    connect(m_fileIsland, &fileIslandWidget::requestCopyMoveDialog, this, [=](const QList<file_location>& targets, bool isMove) {

        qDebug() << "【大总管】收到弹窗请求！准备生成弹窗..."; // 🌟 加一句打印测试
        folderSelectDialog dlg(m_generalControl, m_currentFileLocation, this);

        if (dlg.exec() == QDialog::Accepted) {
            file_location destLoc = dlg.getSelectedLocation();
            auto op = isMove ? filemanager::clipboard_operation::Cut : filemanager::clipboard_operation::Copy;
            m_generalControl->setClipboard(targets, op);
            bool success = m_generalControl->execute_paste(destLoc);
            if (success) {
                qDebug() << "【成功】" << (isMove ? "移动" : "复制") << "完成！";
                navigateTo(m_currentFileLocation);
            }
        } else {
            qDebug() << "用户取消了操作。";
        }
    });

    connect(m_fileIsland, &fileIslandWidget::requestRenameExt, this, [=](const QList<file_location>& targets, const QString& newExt) {

        bool allSuccess = true;
        for (const file_location& target : targets) {
            if (!m_generalControl->change_file_extension(target, newExt)) {
                allSuccess = false;
            }
        }

        if (allSuccess) {
            qDebug() << "【成功】批量改后缀全部完成！";
        } else {
            qDebug() << "【警告】部分文件改后缀失败（可能是被占用或原名无后缀）。";
        }

        navigateTo(m_currentFileLocation);
    });

    connect(m_fileIsland, &fileIslandWidget::requestSystemCopy, this, [=](const QList<file_location>& targets) {

        QList<QUrl> urls;

        for (const file_location& loc : targets) {
            QString absPath = m_generalControl->get_absolute_path(loc.drive, loc.index);
            if (!absPath.isEmpty()) {
                urls.append(QUrl::fromLocalFile(absPath));
            }
        }

        if (!urls.isEmpty()) {
            QMimeData *mimeData = new QMimeData();
            mimeData->setUrls(urls);

            QApplication::clipboard()->setMimeData(mimeData);

            qDebug() << "【系统联动】" << urls.count() << " 个文件已注入系统剪贴板！现在可以直接去微信/资源管理器 Ctrl+V 了！";
        }
    });
    connect(ui->fileDisplayerWidget, &fileDisplayer::onTreemapDoubleClicked, this, [=](uint32_t index, bool isDir) {
        handleFileDoubleClicked("", index, isDir);
    }, Qt::QueuedConnection);
    connect(ui->fileDisplayerWidget, &fileDisplayer::requestTreemapUpdate, this, [=](double w, double h, double exponent) {
        if (m_currentFileLocation.index == INVALID_INDEX || w <= 0 || h <= 0) return;

        std::vector<TreemapItem> mapData = m_generalControl->get_treemap(
            m_currentFileLocation.drive,
            m_currentFileLocation.index,
            w, h, exponent
            );

        ui->fileDisplayerWidget->setTreemapData(mapData, m_currentFileLocation.drive);
    });




}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::handleFileDoubleClicked(QString name, uint32_t index, bool isDir) {
    qDebug() << "【主窗口】捕捉到双击信号！文件名:" << name << " 目录?:" << isDir;

    file_location targetLoc = {m_currentFileLocation.drive, index};

    if (!isDir) {
        QString filePath = m_generalControl->get_absolute_path(targetLoc.drive, targetLoc.index);
        qDebug() << "准备调用系统打开路径：" << filePath;
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        return;
    }
    qDebug() << "【双击下沉】准备进入文件夹:" << name;
    navigateTo(targetLoc);
}



void MainWindow::navigateTo(const file_location& loc) {
    if (loc.index == INVALID_INDEX) return;
    m_currentFileLocation = loc;

    QList<UI_Block> fileDatas = m_generalControl->get_content(loc.drive, loc.index);
    ui->fileDisplayerWidget->setFiles(fileDatas);

    QString currentAbsPath = m_generalControl->get_absolute_path(loc.drive, loc.index);
    ui->fileDisplayerWidget->setCurrentPath(currentAbsPath);

    QList<file_location> rawChain = m_generalControl->get_path_chain(loc);
    QList<QPair<QString, file_location>> breadcrumbData;

    for (const file_location& stepLoc : rawChain) {
        QString folderName = m_generalControl->get_node_name(stepLoc.drive, stepLoc.index);
        // 如果名字是空的，说明它是盘符根节点
        if (folderName.isEmpty()) {
            folderName = stepLoc.drive + ":\\";
        }
        breadcrumbData.append(qMakePair(folderName, stepLoc));
    }

    ui->breadcrumbline->setPath(breadcrumbData);

    double currentW = ui->fileDisplayerWidget->width();
    double currentH = ui->fileDisplayerWidget->height();

    if (currentW > 0 && currentH > 0) {
        std::vector<TreemapItem> mapData = m_generalControl->get_treemap(loc.drive, loc.index, currentW, currentH);
        ui->fileDisplayerWidget->setTreemapData(mapData, loc.drive);
    }
}

void MainWindow::onScanStarted(const QString& drive_letter) {
    ui->fileDisplayerWidget->setFiles(QList<UI_Block>());
    ui->fileDisplayerWidget->setTreemapData(std::vector<TreemapItem>(), "");
    m_currentFileLocation = file_location{"", INVALID_INDEX};
    ui->breadcrumbline->setLabel("正在扫描 " + drive_letter + " 盘...");
    ui->breadcrumbline->setPath(QList<QPair<QString, file_location>>());
    ui->driveZone->setEnabled(false);
    ui->breadcrumbline->setEnabled(false);
}

void MainWindow::onScanFinished(const QString& drive_letter, uint32_t root_index) {
    qDebug() << "【扫描完成】盘符:" << drive_letter << " 根节点:" << root_index;
    m_driveRoots[drive_letter.toUpper()] = root_index;
    ui->driveZone->setEnabled(true);
    ui->breadcrumbline->setEnabled(true);
    ui->breadcrumbline->setLabel("当前路径");

    navigateTo(file_location{drive_letter, root_index});
}

void MainWindow::onScanError(const QString& drive_letter, const QString& error_message){
    // 第一步，清空原有数据
    ui->fileDisplayerWidget->setFiles(QList<UI_Block>());
    ui->fileDisplayerWidget->setTreemapData(std::vector<TreemapItem>(), "");
    // 第二步，恢复刷新和扫描按钮
    ui->driveZone->setEnabled(true);
    ui->breadcrumbline->setEnabled(true);
    // 第三步，报错
    ui->breadcrumbline->setLabel(error_message);
    qDebug() << error_message;
}








