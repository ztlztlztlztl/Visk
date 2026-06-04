cmake_minimum_required(VERSION 3.19)
project(Visk LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Concurrent)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_standard_project_setup()

qt_add_executable(Visk
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    datatype.h
    filemanager.cpp filemanager.h
    general_control.cpp general_control.h
    scanner.cpp scanner.h
    helper.h
    helper.cpp
    constants.h
    breadcrumbwidget.h
    breadcrumbwidget.cpp
    drivelistzone.h
    drivelistzone.cpp
    UsbDeviceMonitor.cpp
    UsbDeviceMonitor.h
    UsbScanner.cpp
    UsbScanner.h
    treemapengine.h
    treemapengine.cpp
)

target_link_libraries(Visk
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::Concurrent
)

install(TARGETS Visk
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Visk
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
