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
    ui->driveList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    // 设置 tableview 的表头
    // 创建
    m_fileDataModel = new QStandardItemModel(this);
    m_fileDataModel->setHorizontalHeaderLabels(QList<QString>{" 名称", " 大小", " 类型", " 修改时间"});
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_fileDataModel);
    ui->filesTableView->setModel(m_proxyModel);
    ui->filesTableView->setSortingEnabled(true);
    // 格式
    QHeaderView *header = ui->filesTableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);           // 名称列：自动拉伸占满大头
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);  // 大小列：根据文字内容多少自动刚好够大
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);  // 类型列：根据文字内容自动刚好够大
    header->setSectionResizeMode(3, QHeaderView::Stretch);           // 修改时间列：也自动拉伸
    // 测试
    appendDriveData(QList<QString>{"test", "111", "111" , "1111-11-11"}, uint32_t(), qint64());
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

void MainWindow::appendDriveData(const QList<QString> &info, const uint32_t &file_index, const qint64 &size)
{
    QList<QStandardItem*> rowItems;

    for (const QString &singleInfo: info){
        rowItems << new QStandardItem(singleInfo);
    }
    rowItems[0]->setData(file_index, Qt::UserRole);
    rowItems[1]->setData(size, Qt::UserRole);
    m_fileDataModel->appendRow(rowItems);
}


void MainWindow::onScanStarted(const QString& drive_letter){
    // 第一步，清空原有数据
    m_fileDataModel->removeRows(0, m_fileDataModel->rowCount());
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
    // 第三步，添加“正在扫描 X 盘”
    appendDriveData(QList<QString>{"正在扫描" + drive_letter + "盘", "", "" , ""}, uint32_t(), qint64());


}
void MainWindow::onScanFinished(const QString& drive_letter, uint32_t root_index){
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
    // 第三步，循环添加信息
    appendDriveData(QList<QString>{ drive_letter + "盘" + "扫描完成", "", "" , ""}, uint32_t(), qint64());
    QList<UI_Block> fileDatas = m_generalControl->get_content(drive_letter, root_index);
    qDebug() << "共" << fileDatas.count() << "个项目";
    for (const UI_Block &singleblock : fileDatas){
        QList<QString> info = QList<QString>();
        info.append(singleblock.file_name);
        info.append(singleblock.is_directory ? "--" : Helper::transToMemory(singleblock.size));
        info.append(singleblock.is_directory ? "文件夹" : "");
        info.append(singleblock.last_modified.toString("yyyy-MM-dd hh:mm:ss"));
        appendDriveData(info, singleblock.file_index, singleblock.size);
    }

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
    // 第三步，报错
    appendDriveData(QList<QString>{ error_message, "", "" , ""}, uint32_t(), qint64());
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

