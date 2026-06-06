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
    "QPushButton { background-color: #F0F0F0; border: 1px solid #D0D0D0; border-radius: 4px; font-weight: bold; color: #333;} "
    "QPushButton:hover { background-color: #E0E0E0; }";
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
    "background-color: #2b2b2b; color: white; border-top: 1px solid #3d3d3d;";
const QString style_fileislandWidget_object_background =
    "background-color: #1e1e1e; border: 1px solid #555; border-radius: 4px;";
const QString style_fileislandWidget_action_button =
    "QPushButton:checked { background-color: #0078D7; border: none; } "
    "QPushButton { background-color: #333; border: 1px solid #555; padding: 6px; border-radius: 4px; }";
const QString style_fileislandWidget_destination_do_button =
    "QPushButton { font-size: 18px; font-weight: bold; background-color: #107C10; border: none; border-radius: 4px; } "
    "QPushButton:disabled { background-color: #555; color: #888; }";
const QString style_fileislandWidget_destination_delete_warning =
    "color: #ff4d4f; font-weight: bold;";
const QString style_fileislandWidget_object_delete_button =
    "QPushButton { background: transparent; border: none; color: #888; font-size: 16px; font-weight: bold; }"
    "QPushButton:hover { color: #ff4d4f; }"
    "QPushButton:pressed { color: #aa0000; }";
const QString style_fileislandWidget_destination_systemcopy_text =
    "color: #00d2ff; font-weight: bold;";
const QString style_fileselectdialog_check_button =
    "background-color: #00d2ff; color: black; font-weight: bold; padding: 5px 15px; border-radius: 3px;";
const QString style_global =
    "QWidget  {color: #1387c0; background-color: #faedd1;};";






























}















#endif // CONSTANTS_H
