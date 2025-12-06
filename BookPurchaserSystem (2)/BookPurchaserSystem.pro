#-------------------------------------------------
#
# Project created by QtCreator 2025-07-21T15:00:00
#
#-------------------------------------------------

QT       += core gui network sql websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BookPurchaserSystem
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# C++11 support
CONFIG += c++11

SOURCES += \
    main.cpp \
    src/BookManager.cpp \
    src/CartManager.cpp \
    src/CategoryManager.cpp \
    src/HttpConnection.cpp \
    src/NetworkConnection.cpp \
    src/NetworkController.cpp \
    src/OrderManager.cpp \
    src/Purchaser.cpp \
    src/TcpConnection.cpp \
    src/UserManager.cpp \
    src/WebSocketConnection.cpp \
    src/LoginDialog.cpp \
    src/BookItemWidget.cpp \
    src/BookDetailWidget.cpp \
    src/MainWindow.cpp

HEADERS += \
    include/BookManager.h \
    include/CartManager.h \
    include/CategoryManager.h \
    include/HttpConnection.h \
    include/NetworkConnection.h \
    include/NetworkController.h \
    include/OrderManager.h \
    include/Purchaser.h \
    include/TcpConnection.h \
    include/UserManager.h \
    include/WebSocketConnection.h \
    include/LoginDialog.h \
    include/BookItemWidget.h \
    include/BookDetailWidget.h \
    include/MainWindow.h

INCLUDEPATH += $$PWD/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Resources are not needed for this project
# RESOURCES += \
#     resources.qrc
