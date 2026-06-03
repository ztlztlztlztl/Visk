#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "helper.h"
#include "general_control.h"
#include "datatype.h"
#include "breadcrumbwidget.h"

#include <QMainWindow>
#include <QLabel>
#include <QEvent>
#include <QEnterEvent>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QDesktopServices>
#include <QUrl>

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


    void onTableDoubleClicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
    // 文件表格
    QStandardItemModel *m_fileDataModel = nullptr;
    QSortFilterProxyModel *m_proxyModel = nullptr;
    void appendfileData(const UI_Block &block);
    void refreshTable(uint32_t targetIndex);
    // 获取文件
    general_control *m_generalControl = nullptr;
    void onScanStarted(const QString& drive_letter);
    void onScanFinished(const QString& drive_letter, uint32_t root_index);
    void onScanError(const QString& drive_letter, const QString& error_message);
};













#endif // MAINWINDOW_H
