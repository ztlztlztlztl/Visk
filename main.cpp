#include "mainwindow.h"
#include "constants.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));


    a.setStyleSheet(Constants::style_global);
    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
