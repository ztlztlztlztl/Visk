#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "helper.h"
#include "general_control.h"
#include "datatype.h"
#include "breadcrumbwidget.h"
#include "filedisplaywidget.h"
#include "fileislandwidget.h"

#include <QMainWindow>
#include <QLabel>
#include <QEvent>
#include <QEnterEvent>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QDesktopServices>
#include <QUrl>
#include <QDockWidget>
#include <QMenu>

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

    void handleFileDoubleClicked(QString name, uint32_t index, bool isDir);

private:
    Ui::MainWindow *ui;
    // 当前文件信息
    file_location m_currentFileLocation;
    void navigateTo(const file_location& loc);
    // 获取文件
    general_control *m_generalControl = nullptr;
    void onScanStarted(const QString& drive_letter);
    void onScanFinished(const QString& drive_letter, uint32_t root_index);
    void onScanError(const QString& drive_letter, const QString& error_message);
    // 文件岛
    QDockWidget *m_dockIsland;
    fileIslandWidget *m_fileIsland;
};













#endif // MAINWINDOW_H
