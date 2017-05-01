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

Client{
    UI_DIR = ./ui

    DEFINES += __CLIENT__

    SOURCES += \
        src/client/*.cpp


    HEADERS += \
        src/client/*.h

    FORMS    +=
}

Server{
    DEFINES += __SERVER__

    SOURCES += \
        src/server/*.cpp

    HEADERS += \
        src/client/*.h

}





