#include "propertydialog.h"

PropertyDialog::PropertyDialog(const UI_Block& info, QWidget *parent)
    : QDialog(parent)
{
    this->setWindowTitle("属性");
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setFixedSize(380, 360);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 15);

    // 顶部标题
    QLabel *titleLabel = new QLabel(info.is_directory ? "文件夹属性" : "文件属性", this);
    titleLabel->setObjectName("titleLabel");
    mainLayout->addWidget(titleLabel);
    // 布局
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setVerticalSpacing(12);
    formLayout->setHorizontalSpacing(15);
    // 填充数据
    formLayout->addRow("名称:", new QLabel(info.file_name));
    formLayout->addRow("绝对路径:", new QLabel(info.absolute_path));
    formLayout->addRow("大小:", new QLabel(Helper::transToMemory(info.size)));
    if (info.is_directory) {
        formLayout->addRow("一级子项数量:", new QLabel(QString::number(info.immediate_file) + " 个项目"));
        formLayout->addRow("全树包含总数:", new QLabel(QString::number(info.total_file) + " 个项目"));
    }
    else{

        formLayout->addRow("修改时间:", new QLabel(info.last_modified.toString("yyyy-MM-dd hh:mm:ss")));
    }

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    mainLayout->addWidget(buttonBox);
}