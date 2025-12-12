QT += core gui widgets

TARGET = purchaser
TEMPLATE = app

# 添加C++11支持
CONFIG += c++11

SOURCES += \
    main.cpp \
    purchaser.cpp \
    book.cpp \
    user.cpp

HEADERS += \
    purchaser.h \
    book.h \
    user.h
