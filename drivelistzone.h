#ifndef DRIVELISTZONE_H
#define DRIVELISTZONE_H

#include "helper.h"
#include "constants.h"



#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QMouseEvent>

class DriveCardWidget : public QWidget{
    Q_OBJECT
public:
    DriveCardWidget(const Helper::DriveInfo &drive, QWidget *parent = nullptr);

signals:
    void cardClicked(const QString &letter);

    void refreshRequested(const QString &letter);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QString m_letter;
    qint64 m_total; // GB
    qint64 m_free; // GB
    QLabel *m_lblTitle;
    QLabel *m_lblDetail;
    QPushButton *m_btnRefresh;

};








class driveListZone : public QWidget {
    Q_OBJECT
public:
    explicit driveListZone(QWidget *parent = nullptr);
    ~driveListZone() = default;

    void updateDriveList(const QList<Helper::DriveInfo> &drives);

signals:
    void globalRefreshRequested();
    void scanDriveRequested(const QString &driveLetter, bool refresh);

private:
    QPushButton *m_btnGlobalRefresh;
    QListWidget *m_listWidget;
};






#endif // DRIVELISTZONE_H
