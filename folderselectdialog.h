#ifndef FOLDERSELECTDIALOG_H
#define FOLDERSELECTDIALOG_H

#include "constants.h"
#include "general_control.h"
#include "breadcrumbwidget.h"
#include "filedisplaywidget.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QObject>

class folderSelectDialog : public QDialog{
    Q_OBJECT
public:
    folderSelectDialog(general_control* gc, file_location startLoc, QWidget* parent = nullptr);

    file_location getSelectedLocation() const { return m_currentLoc; }

private:
    general_control* m_gc;
    file_location m_currentLoc;

    breadcrumbWidget* m_breadcrumb;
    fileDisplayer* m_displayer;

    void navigateTo(const file_location& loc);
};
#endif // FOLDERSELECTDIALOG_H
