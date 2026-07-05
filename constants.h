#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <QString>

namespace Constants{

    // style 区
const QString style_drivecard_lblTitle =
    "color: #0000ff; font-size: 16px; font-weight: bold; background-color: transparent;";
const QString style_drivecard_lblDetail =
    "color: #64748b; font-size: 12px; background-color: transparent;";
const QString style_DriveCardWidget_normal =
    "DriveCardWidget { background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 8px; }";
const QString style_DriveCardWidget_hover =
    "DriveCardWidget { background-color: #e2e8f0; border: 1px solid #cbd5e1; border-radius: 8px; }";
const QString style_DriveCardWidget_clicked =
    "DriveCardWidget { background-color: #f8fafc; border: 2px solid #3b82f6; border-radius: 8px; }";
const QString style_DriveCardWidget_refresh =
    "QPushButton { "
    "  background-color: #EBF3F9; color: #005A9E; border-radius: 4px; border: 1px solid #CCDEED; font-size: 12px;"
    "} "
    "QPushButton:hover { background-color: #DEECF7; }";
const QString style_driveListZone_refresh =
    "QPushButton { background-color: #ffffff; border: 1px solid #D0D0D0; border-radius: 4px; font-weight: bold; color: #333;} "
    "QPushButton:hover { background-color: #e2e8f0; }";
const QString style_breadcrumbWidget_end =
    "QPushButton { border: none; background: transparent; color: black; font-weight: bold; font-size: 14px; }";
const QString style_breadcrumbWidget_normal =
    "QPushButton { border: none; background: transparent; color: #0078D7; font-size: 14px; } "
    "QPushButton:hover { text-decoration: underline; }";
const QString style_breadcrumbWidget_arrow =
    "color: #888888; font-weight: bold;";
const QString style_breadcrumbWidget_text =
    "color: #888888; font-weight: bold;";
const QString style_breadcrumbWidget_background =
    ".breadcrumbWidget{ background-color: #ffffff;}";
const QString style_fileislandWidget_background =
    "background-color: #BCE3C5; color: white; border-top: 1px solid #3d3d3d;";
const QString style_fileislandWidget_object = R"(
    /* ==========================================================================
       全 QSS 控制：终极无边框、无杂线交替列表样式
       ========================================================================== */

    /* 1. 列表大底盘 */
    QListWidget {
        background-color: #E5E7E5;
        alternate-background-color: #F4F7FA; /* 偶数行背景：极其温润舒服的马卡龙淡蓝 */
        border: 1px solid #E2E8F0;           /* 外围只有一圈像空气一样薄的浅灰边框 */
        border-radius: 8px;                  /* 圆角 */
        padding: 4px;
        outline: none;
    }

    /* 2. 每一项（彻底消灭这里的所有边框，靠背景交替色区分足矣） */
    QListWidget::item {
        padding: 0px;                        /* 将内部留白交给 Container 控制，防止间距冲突 */
        border: none;                        /* 💥 锁 1：彻底干掉 Item 自身带来的底边框、顶边框！ */
        background: transparent;
    }

    /* 彻底抹去行本身的原生选中和 Hover 反馈，防止出现系统黑线 */
    QListWidget::item:hover, QListWidget::item:selected {
        background: transparent;
        border: none;                        /* 💥 锁 2：悬浮和选中时也绝对不准产生任何硬线条 */
    }

    /* 3. 核心大招：强行剥离内嵌 Container 的所有样式与边框 */
    QListWidget QWidget {
        background: transparent;
        border: none;                        /* 💥 锁 3：内嵌的组件也全部剥离边框，消灭文字上方的黑线 */
    }

    /* 4. 路径文本样式（使用优雅的深豆沙灰，并加大行高营造网页般的呼吸感） */
    QListWidget QLabel {
        color: #4A5568;
        font-size: 13px;
        font-weight: 500;
        background: transparent;
        border: none;
    }

    /* 5. 减号按钮 */
    QListWidget QPushButton#btnMinus {
        background: transparent;
        border: none;
        color: #A0AEC0;                      /* 平时是低调的浅灰色 “-” */
        font-size: 16px;
        font-weight: bold;
    }

    /* 鼠标移到减号按钮上时，变糖果红 */
    QListWidget QPushButton#btnMinus:hover {
        color: #EE5D5D;
        background-color: rgba(238, 93, 93, 0.1);
        border-radius: 4px;
    }

    QListWidget QPushButton#btnMinus:pressed {
        color: #CF3E3E;
    }
    QScrollBar:vertical { border: none; background: transparent; width: 8px; margin: 0px; }
    QScrollBar::handle:vertical { background: #FFCBA4; border-radius: 4px; min-height: 30px; }
    QScrollBar::handle:vertical:hover { background: #F88379; }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; background: transparent; }
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
)";
const QString style_fileislandWidget_action_button =
    "QPushButton { "
    "color: #4A5568; "
    "background-color: #FFFFFF; "
    "border: 1px solid #E2E8F0; "
    "padding: 6px 14px; "
    "border-radius: 6px; "
    "font-weight: 500; "
    "} "
    "QPushButton:hover { "
    "color: #69C0A0; "
    "border-color: #BCE3C5; "
    "} "
    "QPushButton:checked { "
    "background-color: #A3C6ED; "
    "color: #FFFFFF; "
    "border: none; "
    "font-weight: 600; "
    "}";
const QString style_fileislandWidget_destination_do_button =
    "QPushButton { "
    "font-size: 16px; "
    "font-weight: bold; "
    "background-color: #69C0A0; "
    "color: #FFFFFF; "
    "border: none; "
    "border-radius: 8px; "
    "padding: 6px; "
    "} "
    "QPushButton:hover { "
    "background-color: #58B291; "
    "} "
    "QPushButton:disabled { "
    "background-color: #E2E8F0; "
    "color: #A0AEC0; "
    "}";
const QString style_fileislandWidget_destination_stack =
    "QStackedWidget, QStackedWidget > QWidget { "
    "background-color: #E6F7F0; "
    "border: none; "
    "} "
    "QFrame { border: none; }";
const QString style_fileislandWidget_destination_delete_warning =
    "color: #ff4d4f; font-weight: bold; background-color: transparent;";
const QString style_fileislandWidget_rename_page =
    "QLineEdit { "
    "background-color: rgba(255, 255, 255, 0.6); "
    "border: 1px solid #CBD5E1; "
    "border-radius: 6px; "
    "padding: 4px 10px; "
    "color: #4A5568; "
    "font-size: 13px; "
    "} "
    "QLineEdit:focus { "
    "background-color: #FFFFFF; "
    "border: 1px solid #69C0A0; "
    "} "
    "QLineEdit[text=\"\"] { "
    "color: #A0AEC0; "
    "} "
    "QLabel { "
    "color: #4A5568; "
    "font-size: 13px; "
    "font-weight: 500; "
    "background: transparent; "
    "}";
const QString style_fileislandWidget_object_delete_button =
    "QPushButton { background: transparent; border: none; color: #888; font-size: 16px; font-weight: bold; }"
    "QPushButton:hover { color: #ff4d4f; }"
    "QPushButton:pressed { color: #aa0000; }";
const QString style_fileislandWidget_destination_text =
    "color: #00d2ff; font-weight: bold; background-color: transparent;";
const QString style_fileselectdialog_check_button =
    "QPushButton { "
    "background-color: #A3C6ED; "
    "color: #FFFFFF; "
    "font-weight: 600; "
    "padding: 6px 18px; "
    "border: none; "
    "border-radius: 8px; "
    "} "
    "QPushButton:hover { "
    "background-color: #C0D8F3; "
    "} "
    "QPushButton:pressed { "
    "background-color: #8FAED4; "
    "} ";
const QString style_fileselectdialog_cancel_button =
    "QPushButton { "
    "background-color: #FFDFDF; "
    "color: #EE5D5D; "
    "font-weight: 600; "
    "padding: 6px 18px; "
    "border: none; "
    "border-radius: 8px; "
    "} "
    "QPushButton:hover { "
    "background-color: #FFEBEB; "
    "color: #FF7373; "
    "} "
    "QPushButton:pressed { "
    "background-color: #FFCACA; "
    "color: #CF3E3E; "
    "}";
const QString style_global =
    "QWidget  {color: #1387c0; background-color: #faedd1;}"
    "QDockWidget { titlebar-close-icon: none; titlebar-normal-icon: none; }"
    "QDockWidget::title { background: transparent; height: 0px; padding: 0px; margin: 0px;}"
    "QDialog { background-color: #faedd1; }"
    "QDialog QLabel { color: #4A5568; font-size: 13px; font-family: 'Segoe UI', 'Microsoft YaHei'; }"
    "QDialog QLabel#titleLabel { font-size: 16px; font-weight: bold; color: #2D3748; padding-bottom: 8px; }"
    "QDialog QDialogButtonBox QPushButton { background-color: #69C0A0; color: white; font-weight: bold; border-radius: 6px; padding: 6px 18px; border: none; }"
    "QDialog QDialogButtonBox QPushButton:hover { background-color: #58B291; }";
const QString style_fileisland_call_button =
    "QPushButton#fileislandBtn { "
    "background-color: #FFEADB; "
    "color: #D47A4A; "
    "border: none; "
    "border-radius: 12px; "
    "padding: 6px 20px; "
    "font-weight: 600; "
    "} "
    "QPushButton#fileislandBtn:hover { "
    "background-color: #FFDFCA; "
    "} "
    "QPushButton#fileislandBtn:checked { "
    "background-color: #E6F7F0; "
    "color: #52A885; "
    "} "
    "QPushButton#fileislandBtn:checked:hover { "
    "background-color: #D6F5E6; "
    "} "
    "QPushButton#fileislandBtn:pressed { "
    "padding-top: 8px; "
    "}";
const QString style_filedisplayer_bar  =
    R"(
        QTabWidget::pane {border: none; background: #FFFFFF; border-radius: 12px; }
        QTabBar::tab {
            background: #EAF0F2; color: #718096; border: none; border-radius: 8px; padding: 6px 16px;
            margin-right: 8px; margin-bottom: 4px; font-weight: 500;
        }
        QTabBar::tab:hover { background: #DFE7EB; color: #4A5568; }
        QTabBar::tab:selected { background: #69C0A0; color: #FFFFFF; font-weight: 600; }
    )";
const QString style_filedisplayer_table  =
    R"(
        QHeaderView { background-color: #FFFFFF; border: none; margin: 0px; padding: 0px; }
        QHeaderView::section {
            background-color: #FFFFFF;
            color: #A0AEC0; font-family: "Segoe UI", "Microsoft YaHei", sans-serif;
            font-size: 12px; font-weight: 600;
            padding: 8px 16px;
            border: none;
            border-bottom: 2px solid #EDF2F7;
        }
        QHeaderView::section:hover { background-color: #F7FAFC; color: #69C0A0;}
        QScrollBar:vertical { border: none; background: transparent; width: 12px; margin: 0px; }
        QScrollBar::handle:vertical { background: #E2E8F0; border-radius: 4px; min-height: 30px; }
        QScrollBar::handle:vertical:hover { background: #CBD5E1; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; background: transparent; }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
        QTableView {
            background-color: #FFFFFF; alternate-background-color: #EAFCEB; gridline-color: transparent;
            border: none; outline: none; color: #4A5568;
            font-family: "Segoe UI", "PingFang SC", "Microsoft YaHei", sans-serif; font-size: 13px;
        }
        QTableView::item { padding: 12px 16px; border-bottom: 1px solid #F0F4F6; }
        QTableView::item:hover { background-color: #E6F0FA; color: #2D3748; }
        QTableView::item:selected { background-color: #E6F0FA; color: #2D3748; font-weight: 500; }
    )";

const QString style_filedisplayer_search_combo  =
    R"(
        QComboBox {
            background-color: #FFFFFF;
            border: 1px solid #E2E8F0;
            border-radius: 6px;
            padding: 6px 14px;
            color: #4A5568;
            font-size: 13px;
            font-weight: 500;
        }
        QComboBox:hover, QComboBox:focus {
            border: 1px solid #69C0A0;
            background-color: #F7FAFC;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 24px;
            border-left: none;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 5px solid #A0AEC0;
            margin-right: 8px;
        }
        QComboBox::down-arrow:on {
            border-top: none;
            border-bottom: 5px solid #69C0A0;
        }
        QComboBox QAbstractItemView {
            background-color: #FFFFFF;
            border: 1px solid #E2E8F0;
            border-radius: 0px;
            padding: 0px;
            margin: 0px;
            outline: none;
            selection-background-color: #E6F7F0;
            selection-color: #2D3748;
        }
        QComboBox QAbstractItemView::item {
            color: #4A5568;
            height: 32px;
            padding-left: 10px;
            border: none;
            background: transparent;
        }
        QComboBox QAbstractItemView::item:hover,
        QComboBox QAbstractItemView::item:selected {
            background-color: #E6F7F0;
            color: #2D3748;
            border: none;
            outline: none;
        }
    )";

const QString style_filedisplayer_search_text =
    "QLineEdit { background: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 6px; padding: 6px; color: #4A5568;} QLineEdit:focus { border-color: #69C0A0; }";
const QString style_filedisplayer_search_button =
    "QPushButton { background-color: #69C0A0; color: white; font-weight: bold; border-radius: 6px; padding: 6px 16px; } QPushButton:hover { background-color: #58B291; }";
















}















#endif // CONSTANTS_H
