#define _CRT_RAND_S
#include <stdlib.h>

#include "management.h"
#include "ui_management.h"
#include "charge.h"
#include "configure.h"
#include "form.h"

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
    _updateTimer(new QTimer()),
    ui(new Ui::Management)
{
    ui->setupUi(this); 
    InitWidget();
    InitConnect();
    _updateTimer->start(500);

}

Management::~Management()
{
    delete ui;
    delete _server;
    delete _serverThread;
    delete _updateTimer;
    delete _charge;

}

#define DEFAULT_TEMP 22
#define ROW 2
#define COL 4
void Management::InitWidget() {
    this->setWindowTitle(QString::fromWCharArray(L"中央空调"));
    this->setWindowIcon(QIcon(":/server/fan"));
    ui->tempNumber->display(DEFAULT_TEMP);

    // 默认未打开电源，按钮均无法使用
    ui->tempNumber->setEnabled(false);
    ui->tempUpButton->setEnabled(false);
    ui->tempDownButton->setEnabled(false);
    ui->modeButton->setEnabled(false);

    ui->modeLabel->setPixmap(QPixmap(":/server/cold"));
    _server->setting.setTemperature = 22;
    ui->modeLabel->setEnabled(false);


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

void Management::InitConnect() {
    connect(ui->tempDownButton, &QPushButton::clicked, [this](){
        if(_server->setting.isHeatMode)
            ui->tempNumber->display(std::max(25, ui->tempNumber->intValue() - 1));
        else
            ui->tempNumber->display(std::max(18, ui->tempNumber->intValue() - 1));
        _server->setting.setTemperature = ui->tempNumber->intValue();
    });

    connect(ui->tempUpButton, &QPushButton::clicked, [this](){
        if(_server->setting.isHeatMode)
            ui->tempNumber->display(std::min(30, ui->tempNumber->intValue() + 1));
        else
            ui->tempNumber->display(std::min(25, ui->tempNumber->intValue() + 1));
        _server->setting.setTemperature = ui->tempNumber->intValue();
    });

    connect(ui->checkInButton, &QPushButton::clicked, [this](){
        int roomId;
        try {
            roomId = ui->roomId->currentText().toInt();
        } catch (std::exception) {
            QMessageBox::information(this, "info", "please input digital roomid");
            return;
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

    // 退房结算
    connect(ui->checkOutButton, &QPushButton::clicked, [this](){
        int roomId = ui->roomId->currentText().toInt();
        double power = _server->GetRoomPower(roomId);
        double money = _server->GetRoomMoney(roomId);

        if (!_server->CheckOut(roomId))
            QMessageBox::information(this, "info", "this roomId is not checked in");
        else {
            _labels[roomId].picLabel->setPixmap(QPixmap(":/server/checkout"));

            // 结账界面
            Charge charge(ui->roomId->currentText(), "",
                                 QString::number(power),
                                 QString::number(money));
            charge.exec();
        }

        ui->userIdEdit->clear();
    });

    connect(ui->modeButton, &QPushButton::clicked, [this](){
        if (!_server->setting.isHeatMode) {
            _server->setting.isHeatMode = true;
            _server->setting.setTemperature = 28;
            ui->tempNumber->display(28);
            ui->modeLabel->setPixmap(QPixmap(":/server/warm"));
        } else {
            _server->setting.isHeatMode = false;
            _server->setting.setTemperature = 22;
            ui->tempNumber->display(22);
            ui->modeLabel->setPixmap(QPixmap(":/server/cold"));
        }

    });

    // 配置界面
    connect(ui->configureButton, &QPushButton::clicked, [this](){
       Configure *config = new Configure();
       connect(config, &Configure::change, [this](int maxServe, int frequence){
           _server->setting.maxServe = maxServe;
           _server->setting.frequence = frequence;
       });
       config->exec();
       delete config;
    });


    connect(ui->powerButton, &QPushButton::clicked, [this](){
        _server->setting.isPowerOn = !_server->setting.isPowerOn;
        ui->tempNumber->setEnabled(!ui->tempNumber->isEnabled());
        ui->tempUpButton->setEnabled(!ui->tempUpButton->isEnabled());
        ui->tempDownButton->setEnabled(!ui->tempDownButton->isEnabled());
        ui->modeButton->setEnabled(!ui->modeButton->isEnabled());
        ui->modeLabel->setEnabled(!ui->modeLabel->isEnabled());
    });

    // UI各组件更新
    connect(_updateTimer, &QTimer::timeout, [this](){
        for (const auto& roomId : _roomIds) {
            auto* state = _server->GetRoomState(roomId);
            if (state) {
                _labels[roomId].setTemp->setEnabled(true);
                _labels[roomId].realTemp->setEnabled(true);
                _labels[roomId].fanLabel->setEnabled(true);
                _labels[roomId].setTemp->display(state->setTemperature);
                _labels[roomId].realTemp->display(state->realTemperature);
                if (state->isOn && _server->setting.isPowerOn)
                    _labels[roomId].fanLabel->Start();
                else
                    _labels[roomId].fanLabel->Stop();
            } else {
                _labels[roomId].setTemp->setEnabled(false);
                _labels[roomId].realTemp->setEnabled(false);
                _labels[roomId].fanLabel->setEnabled(false);
                _labels[roomId].setTemp->display(0);
                _labels[roomId].realTemp->display(0);
                _labels[roomId].fanLabel->Stop();
            }
        }
    });

    connect(ui->reportButton, &QPushButton::clicked, [this](){
        Form *form = new Form(_server);
        form->show();
    });
}









