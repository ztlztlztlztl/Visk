/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QListWidget *driveList;
    QPushButton *refreshDriveBtn;
    QFrame *breadcrumbDisplayFrame;
    QHBoxLayout *horizontalLayout_2;
    QLabel *textLabel;
    QFrame *breadcrumbAreaFrame;
    QFrame *frame;
    QGridLayout *gridLayout;
    QTableView *filesTableView;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        driveList = new QListWidget(centralwidget);
        driveList->setObjectName("driveList");
        driveList->setGeometry(QRect(10, 200, 251, 121));
        refreshDriveBtn = new QPushButton(centralwidget);
        refreshDriveBtn->setObjectName("refreshDriveBtn");
        refreshDriveBtn->setGeometry(QRect(40, 170, 80, 18));
        breadcrumbDisplayFrame = new QFrame(centralwidget);
        breadcrumbDisplayFrame->setObjectName("breadcrumbDisplayFrame");
        breadcrumbDisplayFrame->setGeometry(QRect(310, 270, 241, 51));
        breadcrumbDisplayFrame->setFrameShape(QFrame::Shape::StyledPanel);
        breadcrumbDisplayFrame->setFrameShadow(QFrame::Shadow::Raised);
        horizontalLayout_2 = new QHBoxLayout(breadcrumbDisplayFrame);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        textLabel = new QLabel(breadcrumbDisplayFrame);
        textLabel->setObjectName("textLabel");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(textLabel->sizePolicy().hasHeightForWidth());
        textLabel->setSizePolicy(sizePolicy);

        horizontalLayout_2->addWidget(textLabel);

        breadcrumbAreaFrame = new QFrame(breadcrumbDisplayFrame);
        breadcrumbAreaFrame->setObjectName("breadcrumbAreaFrame");
        breadcrumbAreaFrame->setFrameShape(QFrame::Shape::StyledPanel);
        breadcrumbAreaFrame->setFrameShadow(QFrame::Shadow::Raised);

        horizontalLayout_2->addWidget(breadcrumbAreaFrame);

        frame = new QFrame(centralwidget);
        frame->setObjectName("frame");
        frame->setGeometry(QRect(90, 340, 581, 206));
        frame->setFrameShape(QFrame::Shape::StyledPanel);
        frame->setFrameShadow(QFrame::Shadow::Raised);
        gridLayout = new QGridLayout(frame);
        gridLayout->setObjectName("gridLayout");
        filesTableView = new QTableView(frame);
        filesTableView->setObjectName("filesTableView");

        gridLayout->addWidget(filesTableView, 0, 0, 1, 1);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 17));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        refreshDriveBtn->setText(QCoreApplication::translate("MainWindow", "\345\210\267\346\226\260", nullptr));
        textLabel->setText(QCoreApplication::translate("MainWindow", "\345\275\223\345\211\215\350\267\257\345\276\204\357\274\232", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
