#define _CRT_RAND_S
#include <stdlib.h>

#include "management.h"
#include "ui_management.h"

#include "src/include/json.hpp"
#include "src/protocol.h"

#include <QTimer>

using Connor_Socket::Server;
using json = nlohmann::json;


Management::Management(QWidget *parent) :
    QWidget(parent), _server(nullptr),
    ui(new Ui::Management)
{
    ui->setupUi(this);
    std::thread *thread = new std::thread([&](){
        _server = new Server();
    });

    _rooms.emplace(std::make_pair("417", "wangxiaoyu"));
}

Management::~Management()
{
    delete ui;
    delete _server;
    delete thread;
}

void Management::Show() {
    InitWidget();
    InitConnect();
    this->show();
    UpdateSetting();
}

void Management::InitWidget() {
    //expTemp = temp = (int)TempRange::LOWER_BOUND + Random((int)TempRange::UPPER_BOUND - (int)TempRange::LOWER_BOUND);
    //QString t = QString::fromStdString(itos(temp) + " Centigrade");
//    ui->temperature->setText(t);
//    ui->expectedTemp->setText(t);
//    ui->WindSpeed->setText(QString::fromStdString(SpeedStr[(int)speed]));
    //timer = new QTimer();
}

void Management::InitConnect() {
//    QObject::connect(ui->logOutButton, SIGNAL(clicked(bool)), this, SLOT(LogOutClicked()));
//    QObject::connect(ui->tempUp, SIGNAL(clicked(bool)), this, SLOT(TempUpClicked()));
//    QObject::connect(ui->tempDown, SIGNAL(clicked(bool)), this, SLOT(TempDownClicked()));
//    QObject::connect(ui->windUp, SIGNAL(clicked(bool)), this, SLOT(WindUpClicked()));
//    QObject::connect(ui->windDown, SIGNAL(clicked(bool)), this, SLOT(WindDownClicked()));
//    QObject::connect(this->timer, SIGNAL(timeout()), this, SLOT(UpdateTemp()));
}

void Management::UpdateSetting() {
//    bool is_heat_mode = expTemp > temp ? true : false;
//    json sendInfo = {
//        {"op", REQ_UPDATE},
//        {"is_heat_mode", is_heat_mode},
//        {"temp", temp},
//        {"speed", TempInc[(int)speed]}
//    };
//    _client = new Client("");
//    _client->Connect(sendInfo.dump());
//    delete _client;
}

void Management::LogOutClicked() {
    //json sendInfo = {{"op", REQ_STOP}};
    //_client = new Client("");
    //_client->Connect(sendInfo.dump());
    this->close();
    emit toLogIn();
}

void Management::TempUpClicked() {
//    if (this->expTemp != (int)TempRange::UPPER_BOUND) {
//        expTemp++;
//        this->ui->expectedTemp->setText(QString::fromStdString(itos(expTemp) + " Centigrade"));
//        if (!timer->isActive())
//            timer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)speed]);
//        reachExpedtedTemp = false;
////        _client = new Client("");
////        json sendInfo = {{"op", REQ_RESUME}};
////        _client->Connect(sendInfo.dump());
////        delete _client;
//        UpdateSetting();
//    }
}

void Management::TempDownClicked() {
//    if (this->expTemp != (int)TempRange::LOWER_BOUND) {
//        expTemp--;
//        this->ui->expectedTemp->setText(QString::fromStdString(itos(expTemp) + " Centigrade"));
//        if (!timer->isActive())
//            timer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)speed]);
//        reachExpedtedTemp = false;
////        _client = new Client("");
////        json sendInfo = {{"op", REQ_RESUME}};
////        _client->Connect(sendInfo.dump());
////        delete _client;
//        UpdateSetting();
//    }
}

void Management::WindUpClicked() {
//    if (this->speed != Speed::FAST_SPEED) {
//        this->speed = (Speed)(((int)this->speed) + 1);
//        this->ui->WindSpeed->setText(QString::fromStdString(SpeedStr[(int)speed]));
//        if (timer->isActive()) {
//            timer->stop();
//            timer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)speed]);
//        }
//        UpdateSetting();
//    }
}

void Management::WindDownClicked() {
//    if (this->speed != Speed::SLOW_SPEED) {
//        this->speed = (Speed)(((int)this->speed) - 1);
//        this->ui->WindSpeed->setText(QString::fromStdString(SpeedStr[(int)speed]));
//        if (timer->isActive()) {
//            timer->stop();
//            timer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)speed]);
//        }
//        UpdateSetting();
//    }
}

void Management::UpdateTemp() {
//    std::cout << "timeout " << x++ << std::endl;
//    if (expTemp > temp) {
//        temp = temp + 1 > expTemp ? expTemp : temp + 1;
//        this->ui->temperature->setText(QString::fromStdString(itos(temp) + " Centigrade"));
//    }
//    if (expTemp < temp) {
//        temp = temp - 1 < expTemp ? expTemp : temp - 1;
//        this->ui->temperature->setText(QString::fromStdString(itos(temp) + " Centigrade"));
//    }
//    if (!reachExpedtedTemp && expTemp == temp) {
//        reachExpedtedTemp = true;
//        timer->stop();
//        //_client = new Client("");
//        //json sendInfo = {{"op", REQ_STOP}};
//        //_client->Connect(sendInfo.dump());
//        //delete _client;
//    }
}
