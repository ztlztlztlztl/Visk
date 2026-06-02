#include "drivelistzone.h"






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

    this->setFixedHeight(65);

    m_btnRefresh = new QPushButton("重扫", this);
    m_btnRefresh->setFixedSize(60, 26);
    m_btnRefresh->setCursor(Qt::PointingHandCursor);
    m_btnRefresh->setStyleSheet(Constants::style_DriveCardWidget_refresh);

    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->addWidget(m_lblTitle);
    infoLayout->addWidget(m_lblDetail);
    infoLayout->setSpacing(2);
    infoLayout->setAlignment(Qt::AlignVCenter);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 8);

    mainLayout->addLayout(infoLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(m_btnRefresh);

    connect(m_btnRefresh, &QPushButton::clicked, this, [this]() {
        emit refreshRequested(m_letter);
    });

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
        if (m_btnRefresh->geometry().contains(event->pos())) {
            QWidget::mouseReleaseEvent(event);
            return;
        }
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




driveListZone::driveListZone(QWidget *parent)
    : QWidget(parent)
{
    m_btnGlobalRefresh = new QPushButton("刷新设备磁盘", this);
    m_btnGlobalRefresh->setMinimumHeight(38);
    m_btnGlobalRefresh->setStyleSheet(Constants::style_driveListZone_refresh);

    m_listWidget = new QListWidget(this);
    m_listWidget->setFrameShape(QFrame::NoFrame);
    m_listWidget->setStyleSheet("QListWidget { background: transparent; }");
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(8);
    mainLayout->addWidget(m_btnGlobalRefresh);
    mainLayout->addWidget(m_listWidget);

    connect(m_btnGlobalRefresh, &QPushButton::clicked, this, &driveListZone::globalRefreshRequested);
}

void driveListZone::updateDriveList(const QList<Helper::DriveInfo> &drives)
{
    m_listWidget->clear(); // 撕掉旧列表

    for (const Helper::DriveInfo &drive : drives) {
        DriveCardWidget *card = new DriveCardWidget(drive, this);

        // 🌟 经理的职责：接通小弟和老板的电话线
        connect(card, &DriveCardWidget::cardClicked, this, [this](const QString &letter) {
            emit scanDriveRequested(letter, false);
        });

        connect(card, &DriveCardWidget::refreshRequested, this, [this](const QString &letter) {
            emit scanDriveRequested(letter, true);
        });

        QListWidgetItem *item = new QListWidgetItem(m_listWidget);
        item->setSizeHint(card->sizeHint());
        m_listWidget->setItemWidget(item, card);
    }
}

















