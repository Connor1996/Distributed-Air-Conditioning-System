#-------------------------------------------------
#
# Project created by QtCreator 2017-04-29T23:05:00
#
#-------------------------------------------------


Client {
    QT       += core gui
    CONFIG += c++14
}

Server {
    CONFIG += console c++14
    CONFIG -= app_bundle
    CONFIG -= qt
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Distributed-Air-Conditioning-System
TEMPLATE = app

debug {
  DEFINES += __DEBUG__
}

HEADERS += \
        src/socket.h \
        src/protocol.h \
    src/client/conditionorattr.h
Client{
    UI_DIR = ./ui

    DEFINES += __CLIENT__

    SOURCES += \
        src/client/client.cpp \
        src/client/widget.cpp \
        src/client/main.cpp \
        src/client/panel.cpp

    HEADERS += \
        src/client/client.h \
        src/client/widget.h \
        src/include/json.hpp \
        src/client/panel.h

    FORMS    += \
        src/client/widget.ui \
        src/client/panel.ui
}

Server{
    DEFINES += __SERVER__

    SOURCES += \
        src/server/*.cpp

    HEADERS += \
        src/client/*.h

}
