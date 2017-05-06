﻿#define _CRT_RAND_S
#include <stdlib.h>

#include "management.h"
#include "ui_management.h"

#include "src/include/json.hpp"
#include "src/protocol.h"
#include <algorithm>
#include <QMessageBox>
#include <QTimer>

using Connor_Socket::Server;
using json = nlohmann::json;


Management::Management(QWidget *parent) :
    QWidget(parent),
    _thread(new std::thread([this](){
        _server = new Server();
        _server->Start();
    })),
    _roomIds({411, 412, 413, 414, 415, 416, 417, 418}),
    ui(new Ui::Management)
{
    ui->setupUi(this);
    InitWidget();
    InitConnect();
}

Management::~Management()
{
    delete ui;
    delete _server;
    delete _thread;
}

#define DEFAULT_TEMP 18
#define ROW 2
#define COL 4
void Management::InitWidget() {
    ui->tempNumber->display(DEFAULT_TEMP);

    // 默认未打开电源，按钮均无法使用
    ui->tempNumber->setEnabled(false);
    ui->tempUpButton->setEnabled(false);
    ui->tempDownButton->setEnabled(false);
    ui->modeButton->setEnabled(false);


    // 加载房间号
    for (const auto& roomId : _roomIds) {
        ui->roomId->addItem(QString::number(roomId));
    }


    // 把每个房间的图标及信息加载到gridlayout中
    for (auto row = 1; row <= ROW; row++)
    {
        for (auto col = 1; col <= COL; col++) {
            QVBoxLayout *rowlayout = new QVBoxLayout();

            // 构造房间号
            QLabel *textLabel = new QLabel();
            textLabel->setText(QString::number(410 + (row - 1) * COL + col));
            textLabel->setStyleSheet("font: 75 13pt \"Arial Black\"");
            textLabel->setAlignment(Qt::AlignCenter);

            // 构造房间图标
            QLabel *picLabel = new QLabel();
            picLabel->setPixmap(QPixmap(":/server/checkout"));
            picLabel->setAlignment(Qt::AlignCenter);

            // 构造运行图标及温度
            QHBoxLayout *childLayout = new QHBoxLayout();
            QLabel *fanLabel = new QLabel();
            fanLabel->setPixmap(QPixmap(":/server/fan").scaled(30, 30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

            auto createLCDNumber = [](){
                auto tmp = new QLCDNumber(2);
                tmp->setFrameShape(QFrame::NoFrame);
                return tmp;
            };

            QLCDNumber *setTemp = createLCDNumber();
            QLCDNumber *realTemp = createLCDNumber();
            childLayout->addWidget(fanLabel);
            childLayout->addWidget(setTemp);
            childLayout->addWidget(realTemp);

            rowlayout->addWidget(textLabel);
            rowlayout->addWidget(picLabel);
            rowlayout->addLayout(childLayout);
            _labels.emplace(std::make_pair(410 + (row - 1) * COL + col, picLabel));
            ui->gridLayout->addLayout(rowlayout, row, col);
        }
    }
}

#define MIN_TEMP 16
#define MAX_TEMP 30
void Management::InitConnect() {
    connect(ui->tempDownButton, &QPushButton::clicked, [this](){
        ui->tempNumber->display(max(MIN_TEMP, ui->tempNumber->intValue() - 1));
    });

    connect(ui->tempUpButton, &QPushButton::clicked, [this](){
        ui->tempNumber->display(min(MAX_TEMP, ui->tempNumber->intValue() + 1));
    });

    connect(ui->checkInButton, &QPushButton::clicked, [this](){
        int roomId;
        try {
            roomId = ui->roomId->currentText().toInt();
        } catch (std::exception) {
            QMessageBox::information(this, "info", "please input digital roomid");
        }

        std::string userId = ui->userIdEdit->text().toStdString();
        if (_server->CheckIn(roomId, userId)) {
            _labels[roomId]->setPixmap(QPixmap(":/server/checkin"));
            QMessageBox::information(this, "info", "check in successful");
        }
        else
            QMessageBox::information(this, "info", "already check in");

        ui->userIdEdit->clear();
    });

    connect(ui->checkOutButton, &QPushButton::clicked, [this](){
        int roomId = ui->roomId->currentText().toInt();
        if (!_server->CheckOut(roomId))
            QMessageBox::information(this, "info", "this roomId is not checked in");
        else {
            _labels[roomId]->setPixmap(QPixmap(":/server/checkout"));
            QMessageBox::information(this, "info", "check out successful");
        }

        ui->userIdEdit->clear();
    });

    connect(ui->powerButton, &QPushButton::clicked, [this](){
        ui->tempNumber->setEnabled(!ui->tempNumber->isEnabled());
        ui->tempUpButton->setEnabled(!ui->tempUpButton->isEnabled());
        ui->tempDownButton->setEnabled(!ui->tempDownButton->isEnabled());
        ui->modeButton->setEnabled(!ui->modeButton->isEnabled());
    });
}

//void Management::paintEvent(QPaintEvent *) {
//    for (const auto& roomId : _roomIds) {
//        const auto state = _server->GetRoomState(roomId);

//    }
//}








