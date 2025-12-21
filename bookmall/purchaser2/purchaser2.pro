QT += core gui widgets network

TARGET = purchaser
TEMPLATE = app

# 添加C++11支持
CONFIG += c++11

SOURCES += \
    main.cpp \
    purchaser.cpp \
    book.cpp \
    user.cpp \
    tcpclient.cpp \
    apiservice.cpp

HEADERS += \
    purchaser.h \
    book.h \
    user.h \
    tcpclient.h \
    apiservice.h
