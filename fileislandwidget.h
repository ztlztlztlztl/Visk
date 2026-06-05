#ifndef FILEISLANDWIDGET_H
#define FILEISLANDWIDGET_H

#include "constants.h"
#include "datatype.h"

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

class fileIslandWidget : public QWidget {
    Q_OBJECT
public:
    explicit fileIslandWidget(QWidget *parent = nullptr);

    // 底部弹出控制
    void showIsland();
    void hideIsland();
    void switchDrive(const QString &driveLetter);
    // 添加文件
    void addFileToCurrentIsland(QString name, uint32_t index, QString absolutePath);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

signals:
    void requestDelete(const QList<file_location>& targets);
    void requestRenameExt(const QList<file_location>& targets, const QString& newExt);
    void requestCopyMoveDialog(const QList<file_location>& targets, bool isMove);
    void requestSystemCopy(const QList<file_location>& targets);
    void filesDropped(const QList<file_location>& locs);



private slots:
    void onActionToggled(int id);
    void onDoButtonClicked();
    void checkReadyState();
    void onRemoveItemButtonClicked();

private:
    struct file_node{
        file_location fl;
        QString filename;
        QString fileabsolutepath;
    };

    void setupUI();

    // Object
    QListWidget *m_listWidget;

    // Action
    QButtonGroup *m_actionGroup;
    QPushButton *m_btnCopy;
    QPushButton *m_btnMove;
    QPushButton *m_btnDelete;
    QPushButton *m_btnRename;
    QPushButton *m_btnSystemCopy;

    // Destination
    QStackedWidget *m_destinationStack;

    // Page 0：空白
    QWidget *m_pageEmpty;

    // Page 1：复制/移动
    QWidget *m_pagePath;

    // Page 2：删除
    QWidget *m_pageDelete;
    QLabel *m_deleteWarning;

    // Page 3：改后缀
    QWidget *m_pageRename;
    QLineEdit *m_extInput;

    // Page 4：复制到剪贴板
    QWidget *m_pageSystemCopy;

    // 执行按钮
    QPushButton *m_btnDo;
    // 数据
    QString m_currentDrive;
    QMap<QString, QList<file_node>> m_islandData;


    void refreshUI();
};

#endif // FILEISLANDWIDGET_H