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
    // 设置 drivelist
    ui->driveList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    // 设置 breadcrumb
    ui->breadcrumbline->setLabel("等待");
    // 设置 tableview 的表头
    // 创建
    m_fileDataModel = new QStandardItemModel(this);
    m_fileDataModel->setHorizontalHeaderLabels(QList<QString>{" 名称", " 大小", " 类型", " 修改时间"});
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_fileDataModel);
    ui->filesTableView->setModel(m_proxyModel);
    ui->filesTableView->setSortingEnabled(true);
    m_proxyModel->setSortRole(Qt::UserRole + 0);
    ui->filesTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 格式
    QHeaderView *header = ui->filesTableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);           // 名称列：自动拉伸占满大头
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);  // 大小列：根据文字内容多少自动刚好够大
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);  // 类型列：根据文字内容自动刚好够大
    header->setSectionResizeMode(3, QHeaderView::Stretch);           // 修改时间列：也自动拉伸
    ui->filesTableView->verticalHeader()->hide(); // 隐藏序号列
    // 链接函数
    connect(ui->filesTableView, &QTableView::doubleClicked,
            this, &MainWindow::onTableDoubleClicked);
    connect(ui->breadcrumbline, &breadcrumbWidget::pathClicked,
            this, &MainWindow::refreshTable);
    // 扫描所有盘符
    refreshDriveList();
    // 信号接收们
    m_generalControl = new general_control(this);
    connect(m_generalControl, &general_control::scan_started,
            this, &MainWindow::onScanStarted);
    connect(m_generalControl, &general_control::scan_finished,
            this, &MainWindow::onScanFinished);
    connect(m_generalControl, &general_control::scan_error,
            this, &MainWindow::onScanError);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::refreshDriveList(){
    ui->driveList->clear();
    QList<Helper::DriveInfo> drives = Helper::getAllDrives();
    for (const Helper::DriveInfo &drive : drives){
        DriveCardWidget *driveCard = new DriveCardWidget(drive, this);
        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(QSize(200, 70));
        ui->driveList->addItem(item);
        ui->driveList->setItemWidget(item, driveCard);
        // 接受信号
        connect(driveCard, &DriveCardWidget::cardClicked,
                this, [this](const QString &letter){
            m_generalControl->start_scan(letter);
        });
    }
}

void MainWindow::onTableDoubleClicked(const QModelIndex &index)
{
    qDebug() << "捕捉到双击信号";
    if (!index.isValid()) return;

    QModelIndex typeColumnIndex = index.siblingAtColumn(2);
    bool isDir = typeColumnIndex.data(Qt::UserRole + 1).toBool();
    if (!isDir) {
        qDebug() << "双击了普通文件，不执行下沉操作。";
        return;
    }
    QModelIndex nameColumnIndex = index.siblingAtColumn(0);
    uint32_t targetIdx = nameColumnIndex.data(Qt::UserRole + 1).toUInt();
    QString name = nameColumnIndex.data(Qt::DisplayRole).toString();


    qDebug() << "【双击下沉】准备进入文件夹:" << name << " 节点序号:" << targetIdx;

    ui->breadcrumbline->pushNode(name, targetIdx);
    refreshTable(targetIdx);
}

void MainWindow::appendfileData(const UI_Block &block){
    QList<QStandardItem*> rowItems;
    rowItems << new QStandardItem(block.file_name);
    rowItems << new QStandardItem(Helper::transToMemory(block.size));
    rowItems << new QStandardItem(Helper::getFileTypeString(block.file_name, block.is_directory));
    rowItems << new QStandardItem(block.last_modified.toString("yyyy-MM-dd hh:mm:ss"));
    rowItems[0]->setData(block.file_name, Qt::UserRole + 0);
    rowItems[1]->setData(block.size, Qt::UserRole + 0);
    rowItems[2]->setData(Helper::getFileTypeString(block.file_name, block.is_directory), Qt::UserRole + 0);
    rowItems[3]->setData(block.last_modified, Qt::UserRole + 0);
    rowItems[0]->setData(block.file_index, Qt::UserRole + 1);
    rowItems[2]->setData(block.is_directory, Qt::UserRole + 1);
    m_fileDataModel->appendRow(rowItems);
}

void MainWindow::refreshTable(uint32_t targetIndex){
    m_fileDataModel->removeRows(0, m_fileDataModel->rowCount());


    QString driveLetter = ui->breadcrumbline->m_pathStack.first().first.left(1);

    QList<UI_Block> fileDatas = m_generalControl->get_content(driveLetter, targetIndex);
    qDebug() << " 共找到:" << fileDatas.count() << "个子项";

    for (const UI_Block &block : fileDatas) {
        appendfileData(block);
    }
    m_proxyModel->sort(0, Qt::AscendingOrder);
}

void MainWindow::onScanStarted(const QString& drive_letter){
    // 第一步，清空原有数据
    m_fileDataModel->removeRows(0, m_fileDataModel->rowCount());
    ui->breadcrumbline->setLabel("正在扫描" + drive_letter + "盘");
    ui->breadcrumbline->initRoot();
    // 第二步，锁死刷新和扫描按钮
    ui->refreshDriveBtn->setEnabled(false);
    int itemCount = ui->driveList->count();
    for (int i = 0; i < itemCount;++i){
        DriveCardWidget *card =
            qobject_cast<DriveCardWidget*>(ui->driveList->itemWidget(ui->driveList->item(i)));
        if (card){
            card->setEnabled(false);
        }
    }
    ui->breadcrumbline->setEnabled(false);


}

void MainWindow::onScanFinished(const QString& drive_letter, uint32_t root_index){
    qDebug() << "finished";
    // 第一步，清空原有数据
    m_fileDataModel->removeRows(0, m_fileDataModel->rowCount());
    ui->breadcrumbline->setLabel("当前路径");
    ui->breadcrumbline->initRoot();
    ui->breadcrumbline->pushNode(drive_letter + "盘", root_index);
    // 第二步，恢复刷新和扫描按钮
    ui->refreshDriveBtn->setEnabled(true);
    int itemCount = ui->driveList->count();
    for (int i = 0; i < itemCount;++i){
        DriveCardWidget *card =
            qobject_cast<DriveCardWidget*>(ui->driveList->itemWidget(ui->driveList->item(i)));
        if (card){
            card->setEnabled(true);
        }
    }
    ui->breadcrumbline->setEnabled(true);
    // 第三步，循环添加信息
    refreshTable(root_index);

}

void MainWindow::onScanError(const QString& drive_letter, const QString& error_message){
    // 第一步，清空原有数据
    m_fileDataModel->removeRows(0, m_fileDataModel->rowCount());
    // 第二步，恢复刷新和扫描按钮
    ui->refreshDriveBtn->setEnabled(true);
    int itemCount = ui->driveList->count();
    for (int i = 0; i < itemCount;++i){
        DriveCardWidget *card =
            qobject_cast<DriveCardWidget*>(ui->driveList->itemWidget(ui->driveList->item(i)));
        if (card){
            card->setEnabled(true);
        }
    }
    ui->breadcrumbline->setEnabled(true);
    // 第三步，报错
    qDebug() << error_message;
}






DriveCardWidget::DriveCardWidget(const Helper::DriveInfo &drive, QWidget *parent)
    : QWidget(parent)
{
    this->setAttribute(Qt::WA_StyledBackground, true);
    m_letter = drive.letter;
    m_total = drive.totalBytes;
    m_free = drive.freeBytes;

    m_lblTitle = new QLabel(drive.letter + " 盘  (" + drive.name + ")", this);
    double totalGB = m_total / (1024.0 * 1024.0 * 1024.0);
    double freeGB =  m_free/ (1024.0 * 1024.0 * 1024.0);
    QString totalStr = QString::number(totalGB, 'f', 1) + " GB";
    QString freeStr = QString::number(freeGB, 'f', 1) + " GB";
    m_lblDetail = new QLabel(freeStr + "/" + totalStr, this);

    m_lblTitle->setStyleSheet(Constants::style_drivecard_lblTitle);
    m_lblDetail->setStyleSheet(Constants::style_drivecard_lblDetail);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_lblTitle);
    layout->addWidget(m_lblDetail);
    layout->setContentsMargins(16, 12, 16, 12); // 设置四周留白

    this->setStyleSheet(Constants::style_DriveCardWidget_normal);
}

void DriveCardWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        this->setStyleSheet(Constants::style_DriveCardWidget_clicked);
    } else {
        QWidget::mousePressEvent(event);
    }
}

void DriveCardWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->setStyleSheet(Constants::style_DriveCardWidget_hover);
        emit cardClicked(m_letter); // 触发信号
        event->accept();
    }
    QWidget::mouseReleaseEvent(event);
}

void DriveCardWidget::enterEvent(QEnterEvent *event)
{
    // 鼠标放上去时变成浅灰色
    this->setStyleSheet(Constants::style_DriveCardWidget_hover);
    QWidget::enterEvent(event);
}

void DriveCardWidget::leaveEvent(QEvent *event)
{
    // 鼠标恢复原状
    this->setStyleSheet(Constants::style_DriveCardWidget_normal);
    QWidget::leaveEvent(event);
}



void MainWindow::on_refreshDriveBtn_clicked()
{
    refreshDriveList();
}

