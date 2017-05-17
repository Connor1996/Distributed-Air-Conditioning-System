#define _CRT_RAND_S
#include <stdlib.h>

#include "management.h"
#include "ui_management.h"
#include "charge.h"

#include "src/include/json.hpp"
#include "src/protocol.h"
#include "rotationlabel.h"

#include <QMessageBox>
#include <QPropertyAnimation>
#include <QGraphicsView>

using Connor_Socket::Server;
using json = nlohmann::json;


Management::Management(QWidget *parent) :
    QWidget(parent),
    _roomIds({411, 412, 413, 414, 415, 416, 417, 418}),
    _serverThread(new std::thread([this](){
        _server = new Server();
        _server->Start();
    })),
    _updateThread(nullptr),
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
    delete _serverThread;
    delete _updateThread;
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

            RotationLabel *fanLabel = new RotationLabel();
            fanLabel->setAlignment(Qt::AlignCenter);
            fanLabel->setEnabled(false);

            auto createLCDNumber = [](){
                auto tmp = new QLCDNumber(2);
                tmp->setFrameShape(QFrame::NoFrame);
                tmp->setSegmentStyle(QLCDNumber::SegmentStyle::Flat);
                tmp->setEnabled(false);
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
            rowlayout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
            _labels.emplace(std::make_pair(410 + (row - 1) * COL + col,
                                           RoomLabels{picLabel, fanLabel, setTemp, realTemp}));
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
            _labels[roomId].picLabel->setPixmap(QPixmap(":/server/checkin"));
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
            _labels[roomId].picLabel->setPixmap(QPixmap(":/server/checkout"));
            //QMessageBox::information(this, "info", "check out successful");
            room_id = QString::number(roomId,10);
            cost = new charge(room_id,user_id,total_time,total_money);
            cost->show();
        }

        ui->userIdEdit->clear();
    });

    connect(ui->modeButton, &QPushButton::clicked, [this](){
        if (ui->modeLabel->text() == "cold") {
            _server->_setting.isHeatMode = true;
            ui->modeLabel->setText("heat");
        } else {
            _server->_setting.isHeatMode = false;
            ui->modeLabel->setText("cold");
        }

    });


    connect(ui->powerButton, &QPushButton::clicked, [this](){
        _server->_setting.isPowerOn = !_server->_setting.isPowerOn;
        ui->tempNumber->setEnabled(!ui->tempNumber->isEnabled());
        ui->tempUpButton->setEnabled(!ui->tempUpButton->isEnabled());
        ui->tempDownButton->setEnabled(!ui->tempDownButton->isEnabled());
        ui->modeButton->setEnabled(!ui->modeButton->isEnabled());

        if (ui->tempNumber->isEnabled()) {
            if (_updateThread) {
                 _updateThread->join();
                delete _updateThread;
            }

            _updateThread = new std::thread([this](){
                while(true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    if (!ui->tempNumber->isEnabled())
                        return;
                    for (const auto& roomId : _roomIds) {
                        auto* state = _server->GetRoomState(roomId);
                        if (state) {
//                            _labels[roomId].setTemp->setEnabled(true);
//                            _labels[roomId].realTemp->setEnabled(true);
//                            _labels[roomId].fanLabel->setEnabled(true);
                            _labels[roomId].setTemp->display(state->setTemperature);
                            _labels[roomId].realTemp->display(state->realTemperature);
                            if (state->isOn && _server->_setting.isPowerOn)
                                _labels[roomId].fanLabel->Start();
                            else
                                _labels[roomId].fanLabel->Stop();
                        } else {
//                            _labels[roomId].setTemp->setEnabled(false);
//                            _labels[roomId].realTemp->setEnabled(false);
//                            _labels[roomId].fanLabel->setEnabled(false);
                            _labels[roomId].setTemp->display(0);
                            _labels[roomId].realTemp->display(0);
                            _labels[roomId].fanLabel->Stop();
                        }
                    }
                }
            });
        } else {
            for (const auto& roomId : _roomIds) {
//                _labels[roomId].setTemp->setEnabled(false);
//                _labels[roomId].realTemp->setEnabled(false);
//                _labels[roomId].fanLabel->setEnabled(false);
                _labels[roomId].setTemp->display(0);
                _labels[roomId].realTemp->display(0);
                _labels[roomId].fanLabel->Stop();
            }
        }
    });
}









