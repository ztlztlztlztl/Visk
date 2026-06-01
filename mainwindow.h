#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "helper.h"
#include "general_control.h"
#include "datatype.h"

#include <QMainWindow>
#include <QLabel>
#include <QEvent>
#include <QEnterEvent>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:

    void on_refreshDriveBtn_clicked();

private:
    Ui::MainWindow *ui;
    // 生成drivelist
    void refreshDriveList();
    // 文件表格
    QStandardItemModel *m_fileDataModel = nullptr;
    QSortFilterProxyModel *m_proxyModel = nullptr;
    void appendDriveData(const QList<QString> &info, const uint32_t &file_index, const qint64 &size);
    // 获取文件
    general_control *m_generalControl = nullptr;
    void onScanStarted(const QString& drive_letter);
    void onScanFinished(const QString& drive_letter, uint32_t root_index);
    void onScanError(const QString& drive_letter, const QString& error_message);
};





class DriveCardWidget : public QWidget{
    Q_OBJECT
public:
    DriveCardWidget(const Helper::DriveInfo &drive, QWidget *parent = nullptr);

signals:
    void cardClicked(const QString &letter);

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

};








#endif // MAINWINDOW_H
