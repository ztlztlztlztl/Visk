#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include "datatype.h"
#include "helper.h"

#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>



class PropertyDialog : public QDialog {
    Q_OBJECT
public:
    explicit PropertyDialog(const UI_Block& info, QWidget *parent = nullptr);
};

#endif // PROPERTYDIALOG_H
