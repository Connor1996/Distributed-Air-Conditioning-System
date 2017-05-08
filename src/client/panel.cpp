#define _CRT_RAND_S
#include<stdlib.h>

#include "panel.h"
#include "ui_panel.h"

#include <sstream>
#include "src/include/json.hpp"
#include "src/protocol.h"
#include "conditionorattr.h"

#include <QTimer>
#include <QMessageBox>
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

Panel::Panel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Panel)
{
    ui->setupUi(this);
}

Panel::~Panel()
{
    delete ui;
    delete tempTimer;
    delete notifyTimer;
    delete recoveryTimer;
    delete _client;
}

void Panel::InitWidget() {
    ca.onoff = Switch::OFF;
    ca.is_heat_mode = false;
    ca.speed = Speed::NORMAL_SPEED;
    ca.expTemp = ca.temp = ca.original_temp = (int)TempRange::LOWER_BOUND +
            Random((int)TempRange::UPPER_BOUND - (int)TempRange::LOWER_BOUND);
    tempTimer = new QTimer();
    notifyTimer = new QTimer();
    recoveryTimer = new QTimer();
}

void Panel::InitConnect() {
    QObject::connect(ui->logOutButton, SIGNAL(clicked(bool)), this, SLOT(LogOutClicked()));
    QObject::connect(ui->tempUp, SIGNAL(clicked(bool)), this, SLOT(TempUpClicked()));
    QObject::connect(ui->tempDown, SIGNAL(clicked(bool)), this, SLOT(TempDownClicked()));
    QObject::connect(ui->windUp, SIGNAL(clicked(bool)), this, SLOT(WindUpClicked()));
    QObject::connect(ui->windDown, SIGNAL(clicked(bool)), this, SLOT(WindDownClicked()));
    QObject::connect(ui->modeButton, SIGNAL(clicked(bool)), this, SLOT(ModeClicked()));
    QObject::connect(ui->switchButton, SIGNAL(clicked(bool)), this, SLOT(SwitchClicked()));
    QObject::connect(this->tempTimer, SIGNAL(timeout()), this, SLOT(AdjustTemp()));
    QObject::connect(this->notifyTimer, SIGNAL(timeout()), this, SLOT(ReportState()));
    QObject::connect(this->recoveryTimer, SIGNAL(timeout()), this, SLOT(RecoverTemp()));
}

void Panel::UpdateRequest() {
    ca.is_heat_mode = ca.expTemp > ca.temp ? true : false;
    json sendInfo = {
        {"op", REQ_UPDATE},
        {"is_heat_mode", ca.is_heat_mode},
        {"temp", ca.temp},
        {"speed", TempInc[(int)ca.speed]}
    };
    _client->Send(sendInfo.dump());
}

void Panel::DisableItems() {
    ui->controller->setEnabled(false);
    ui->logOutButton->setEnabled(true);
    ui->switchButton->setEnabled(true);
    tempTimer->stop();
    notifyTimer->stop();
    recoveryTimer->stop();
}


void Panel::EnableItems() {
    ui->controller->setEnabled(true);
    ui->windSpeed->setText(QString::fromStdString(SpeedStr[(int)ca.speed]));
    ui->mode->setText(QString::fromWCharArray(L"制冷"));
    QString t = QString::number(ca.temp) + " Centigrade";
    ui->temperature->setText(t);
    ui->expectedTemp->setText(t);
    ui->originalTemp->setText(t);
    notifyTimer->start(NOTIFY_PERIOD);
}

void Panel::Show(Connor_Socket::Client* c) {
    _client = c;
    InitWidget();
    InitConnect();
    this->show();
}

void Panel::LogOutClicked() {
    json sendInfo = {{"op", REQ_STOP}};
    _client->Send(sendInfo.dump());
    delete tempTimer, notifyTimer, recoveryTimer;
    tempTimer = notifyTimer = recoveryTimer = NULL;
    this->close();
    emit toLogIn();
}

void Panel::TempUpClicked() {
    if (this->ca.expTemp != (int)TempRange::UPPER_BOUND) {
        ca.expTemp++;
        this->ui->expectedTemp->setText(QString::number(ca.expTemp) + " Centigrade");
        if (!tempTimer->isActive() &&
                (ca.is_heat_mode && ca.temp < ca.expTemp || !ca.is_heat_mode && ca.temp > ca.expTemp))
            tempTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)ca.speed]);
        json sendInfo = {{"op", REQ_RESUME}};
        _client->Send(sendInfo.dump());
        UpdateRequest();
    }
}

void Panel::TempDownClicked() {
    if (this->ca.expTemp != (int)TempRange::LOWER_BOUND) {
        ca.expTemp--;
        this->ui->expectedTemp->setText(QString::number(ca.expTemp) + " Centigrade");
        if (!tempTimer->isActive() &&
                (ca.is_heat_mode && ca.temp < ca.expTemp || !ca.is_heat_mode && ca.temp > ca.expTemp))
            tempTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)ca.speed]);
        json sendInfo = {{"op", REQ_RESUME}};
        _client->Send(sendInfo.dump());
        UpdateRequest();
    }
}

void Panel::WindUpClicked() {
    if (this->ca.speed != Speed::FAST_SPEED) {
        this->ca.speed = (Speed)(((int)this->ca.speed) + 1);
        this->ui->windSpeed->setText(QString::fromStdString(SpeedStr[(int)ca.speed]));
        if (tempTimer->isActive()) {
            tempTimer->stop();
            tempTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)ca.speed]);
        }
        UpdateRequest();
    }
}

void Panel::WindDownClicked() {
    if (this->ca.speed != Speed::SLOW_SPEED) {
        this->ca.speed = (Speed)(((int)this->ca.speed) - 1);
        this->ui->windSpeed->setText(QString::fromStdString(SpeedStr[(int)ca.speed]));
        if (tempTimer->isActive()) {
            tempTimer->stop();
            tempTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)ca.speed]);
        }
        UpdateRequest();
    }
}

void Panel::ModeClicked() {
    ca.is_heat_mode = !ca.is_heat_mode;
    UpdateRequest();
    this->ui->mode->setText(QString::fromWCharArray(ca.is_heat_mode ? L"制热" : L"制冷"));
}

void Panel::SwitchClicked() {
    if (ca.onoff == Switch::OFF) {
        ca.onoff = Switch::ON;
        EnableItems();
        json sendInfo = {{"op", REQ_RESUME}};
        _client->Send(sendInfo.dump());
        UpdateRequest();
    }
    else {
        ca.onoff = Switch::OFF;
        DisableItems();
        json sendInfo = {{"op", REQ_STOP}};
        _client->Send(sendInfo.dump());
        recoveryTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)Speed::NORMAL_SPEED]);
    }
}

void Panel::AdjustTemp() {
    if (ca.expTemp > ca.temp) {
        ca.temp = ca.temp + 1 > ca.expTemp ? ca.expTemp : ca.temp + 1;
        this->ui->temperature->setText(QString::number(ca.temp) + " Centigrade");
    }
    if (ca.expTemp < ca.temp) {
        ca.temp = ca.temp - 1 < ca.expTemp ? ca.expTemp : ca.temp - 1;
        this->ui->temperature->setText(QString::number(ca.temp) + " Centigrade");
    }
    if (ca.expTemp == ca.temp) {
        if (tempTimer->isActive())
            tempTimer->stop();
        json sendInfo = {{"op", REQ_STOP}};
        _client->Send(sendInfo.dump());
        recoveryTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)Speed::NORMAL_SPEED]);
    }
}

void Panel::ReportState() {
    json sendInfo = {
        {"op", REPORT_STATE},
        {"set_temp", ca.expTemp},
        {"real_temp", ca.temp},
        {"speed", TempInc[(int)ca.speed]}
    };
    _client->Send(sendInfo.dump());
}

void Panel::RecoverTemp() {
    //温度恢复到可以忍受的最大范围
    if (ca.onoff == Switch::ON && abs(ca.expTemp - ca.temp) == TEMP_BEAR_RANGE) {
        recoveryTimer->stop();
        tempTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)ca.speed]);
        json sendInfo = {{"op", REQ_RESUME}};
        _client->Send(sendInfo.dump());
    }
    else if (ca.temp == ca.original_temp) {
        recoveryTimer->stop();
    }
    else {
        if (ca.temp < ca.original_temp) {
            ca.temp = ca.temp + 1 > ca.original_temp ? ca.original_temp : ca.temp + 1;
            this->ui->temperature->setText(QString::number(ca.temp) + " Centigrade");
        }
        if (ca.temp > ca.original_temp) {
            ca.temp = ca.temp - 1 < ca.original_temp ? ca.original_temp : ca.temp - 1;
            this->ui->temperature->setText(QString::number(ca.temp) + " Centigrade");
        }
    }
}
