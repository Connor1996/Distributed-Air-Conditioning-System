#define _CRT_RAND_S
#include<stdlib.h>

#include "panel.h"
#include "ui_panel.h"

#include <sstream>
#include "src/include/json.hpp"
#include "src/protocol.h"
#include "conditionorattr.h"

#include <QTimer>
#include <thread>

using Connor_Socket::Client;
using json = nlohmann::json;

unsigned int Random(int max)
{
   errno_t err;
   unsigned int number;
   err = rand_s(&number);
   if (err != 0)
   {
        return 0;
   }
   return (unsigned int)((double)number / ((double)UINT_MAX + 1) * double(max)) + 1;
}

std::string itos(int n) {
    std::stringstream ss;
    ss << n;
    std::string s;
    ss >> s;
    return s;
}

Panel::Panel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Panel)
{
    ui->setupUi(this);
}

Panel::~Panel()
{
    delete ui;
    delete tempTimer, notifyTimer;
    if (_client != NULL) {
        delete _client;
        _client = NULL;
    }
}

void Panel::Show(Connor_Socket::Client* c) {
    _client = c;
    InitWidget();
    InitConnect();
    this->show();
    UpdateSetting();
}

void Panel::InitWidget() {
    ca.expTemp = ca.temp = (int)TempRange::LOWER_BOUND + Random((int)TempRange::UPPER_BOUND - (int)TempRange::LOWER_BOUND);
    ca.speed = Speed::NORMAL_SPEED;
    QString t = QString::fromStdString(itos(ca.temp) + " Centigrade");
    ui->temperature->setText(t);
    ui->expectedTemp->setText(t);
    ui->WindSpeed->setText(QString::fromStdString(SpeedStr[(int)ca.speed]));
    tempTimer = new QTimer();
    notifyTimer = new QTimer();
    notifyTimer->start(1000);
}

void Panel::InitConnect() {
    QObject::connect(ui->logOutButton, SIGNAL(clicked(bool)), this, SLOT(LogOutClicked()));
    QObject::connect(ui->tempUp, SIGNAL(clicked(bool)), this, SLOT(TempUpClicked()));
    QObject::connect(ui->tempDown, SIGNAL(clicked(bool)), this, SLOT(TempDownClicked()));
    QObject::connect(ui->windUp, SIGNAL(clicked(bool)), this, SLOT(WindUpClicked()));
    QObject::connect(ui->windDown, SIGNAL(clicked(bool)), this, SLOT(WindDownClicked()));
    QObject::connect(this->tempTimer, SIGNAL(timeout()), this, SLOT(UpdateTemp()));
    QObject::connect(this->notifyTimer, SIGNAL(timeout()), this, SLOT(UpdateSetting()));
}

void Panel::UpdateSetting() {
    ca.is_heat_mode = ca.expTemp > ca.temp ? true : false;
    json sendInfo = {
        {"op", REQ_UPDATE},
        {"is_heat_mode", ca.is_heat_mode},
        {"temp", ca.temp},
        {"speed", TempInc[(int)ca.speed]}
    };
    _client->Send(sendInfo.dump());
}

void Panel::LogOutClicked() {
    json sendInfo = {{"op", REQ_STOP}};
    _client->Send(sendInfo.dump());
    this->close();
    emit toLogIn();
}

void Panel::TempUpClicked() {
    if (this->ca.expTemp != (int)TempRange::UPPER_BOUND) {
        ca.expTemp++;
        this->ui->expectedTemp->setText(QString::fromStdString(itos(ca.expTemp) + " Centigrade"));
        if (!tempTimer->isActive())
            tempTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)ca.speed]);
        json sendInfo = {{"op", REQ_RESUME}};
        _client->Send(sendInfo.dump());
    }
}

void Panel::TempDownClicked() {
    if (this->ca.expTemp != (int)TempRange::LOWER_BOUND) {
        ca.expTemp--;
        this->ui->expectedTemp->setText(QString::fromStdString(itos(ca.expTemp) + " Centigrade"));
        if (!tempTimer->isActive())
            tempTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)ca.speed]);
        json sendInfo = {{"op", REQ_RESUME}};
        _client->Send(sendInfo.dump());
    }
}

void Panel::WindUpClicked() {
    if (this->ca.speed != Speed::FAST_SPEED) {
        this->ca.speed = (Speed)(((int)this->ca.speed) + 1);
        this->ui->WindSpeed->setText(QString::fromStdString(SpeedStr[(int)ca.speed]));
        if (tempTimer->isActive()) {
            tempTimer->stop();
            tempTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)ca.speed]);
        }
    }
}

void Panel::WindDownClicked() {
    if (this->ca.speed != Speed::SLOW_SPEED) {
        this->ca.speed = (Speed)(((int)this->ca.speed) - 1);
        this->ui->WindSpeed->setText(QString::fromStdString(SpeedStr[(int)ca.speed]));
        if (tempTimer->isActive()) {
            tempTimer->stop();
            tempTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)ca.speed]);
        }
    }
}

void Panel::UpdateTemp() {
    if (ca.expTemp > ca.temp) {
        ca.temp = ca.temp + 1 > ca.expTemp ? ca.expTemp : ca.temp + 1;
        this->ui->temperature->setText(QString::fromStdString(itos(ca.temp) + " Centigrade"));
    }
    if (ca.expTemp < ca.temp) {
        ca.temp = ca.temp - 1 < ca.expTemp ? ca.expTemp : ca.temp - 1;
        this->ui->temperature->setText(QString::fromStdString(itos(ca.temp) + " Centigrade"));
    }
    if (ca.expTemp == ca.temp) {
        if (tempTimer->isActive())
            tempTimer->stop();
        json sendInfo = {{"op", REQ_STOP}};
        _client->Send(sendInfo.dump());
    }
}
