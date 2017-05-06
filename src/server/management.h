﻿#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include "server.h"

#include <QWidget>
#include <QLabel>
#include <QLCDNumber>

#include <unordered_map>

namespace Ui {
class Management;
}

class Management : public QWidget
{
    Q_OBJECT

public:
    explicit Management(QWidget *parent = 0);
    ~Management();

private:
    void InitWidget();
    void InitConnect();

    Ui::Management *ui;
    Connor_Socket::Server *_server;
    std::thread *_serverThread;
    std::thread *_updateThread;

    struct RoomLabels {
        QLabel *picLabel;
        QLabel *fanLabel;
        QLCDNumber *setTemp;
        QLCDNumber *realTemp;
    };

    std::unordered_map<int, struct RoomLabels> _labels;
    std::list<int> _roomIds;
};

#endif // MANAGEMENT_H
