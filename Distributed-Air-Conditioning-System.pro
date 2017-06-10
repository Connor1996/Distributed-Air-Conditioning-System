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
    QT       += core gui
    QT += charts
    CONFIG += c++14
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
        src/include/json.hpp \
        src/rotationlabel.h \
        src/server/configure.h \
        src/model/roominfo.h \
        src/model/roomrequest.h


UI_DIR = ./ui

Client{

    DEFINES += __CLIENT__

    SOURCES += \
        src/client/client.cpp \
        src/client/widget.cpp \
        src/client/main.cpp \
        src/client/panel.cpp

    HEADERS += \
        src/client/client.h \
        src/client/widget.h \
        src/client/panel.h \
        src/client/conditionorattr.h

    FORMS    += \
        src/client/widget.ui \
        src/client/panel.ui
}

Server{
    DEFINES += __SERVER__

    SOURCES += \
        src/server/dispatch.cpp \
        src/server/server.cpp \
        src/server/main.cpp \
        src/include/sqlite3.c \
        src/server/login.cpp \
        src/server/management.cpp \
        src/server/form.cpp


    HEADERS += \
        src/server/dispatch.h \
        src/server/ormlite.h \
        src/server/server.h \
        src/include/sqlite3.h \
        src/server/login.h \
        src/server/management.h \
        src/server/charge.h \
        src/server/request.h \
        src/server/form.h

    FORMS += \
        src/server/login.ui \
        src/server/management.ui \
        src/server/charge.ui \
        src/server/configure.ui \
        src/server/form.ui
}

RESOURCES += \
    image/image.qrc




