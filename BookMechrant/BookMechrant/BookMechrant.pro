QT += core gui network widgets charts
TARGET = BookMerchant
TEMPLATE = app
CONFIG += c++14

# 源文件列表
SOURCES += \
    main.cpp \
    Seller.cpp \
    LoginWidget.cpp \
    DashboardWidget.cpp \
    BookManageWidget.cpp \
    OrderManageWidget.cpp \
    MemberWidget.cpp \
    ReportWidget.cpp \
    NetworkClient.cpp \
    systemmanagewidget.cpp

# 头文件列表
HEADERS += \
    Seller.h \
    LoginWidget.h \
    DashboardWidget.h \
    BookManageWidget.h \
    OrderManageWidget.h \
    MemberWidget.h \
    ReportWidget.h \
    NetworkClient.h \
    systemmanagewidget.h

# 编译输出目录
DESTDIR = $$PWD/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc

# 包含路径
INCLUDEPATH += $$PWD

# 调试和发布配置
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,_debug)
    DEFINES += DEBUG_MODE
} else {
    DEFINES += RELEASE_MODE QT_NO_DEBUG_OUTPUT
}
