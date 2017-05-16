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
    delete sendTimer;
    delete _client;
}

void Panel::InitWidget() {
    ca.is_on = false;
    ca.is_heat_mode = false;
    ca.speed = Speed::NORMAL_SPEED;
    ca.expTemp = ca.temp = ca.original_temp = (int)TempRange::LOWER_BOUND +
            Random((int)TempRange::UPPER_BOUND - (int)TempRange::LOWER_BOUND);
    tempTimer = new QTimer();
    notifyTimer = new QTimer();
    recoveryTimer = new QTimer();
    sendTimer = new QTimer();
    DisableItems();
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
    QObject::connect(this->sendTimer, SIGNAL(timeout()), this, SLOT(ClusterSend()));
}


void Panel::DisableItems() {
    ui->tempUp->setEnabled(false);
    ui->tempDown->setEnabled(false);
    ui->windUp->setEnabled(false);
    ui->windDown->setEnabled(false);
    ui->modeButton->setEnabled(false);
    ui->logOutButton->setEnabled(true);
    ui->switchButton->setEnabled(true);
    if (tempTimer->isActive())
        tempTimer->stop();
    if (notifyTimer->isActive())
        notifyTimer->stop();
    if (recoveryTimer->isActive())
        recoveryTimer->stop();
    if (sendTimer->isActive())
        sendTimer->stop();
}


void Panel::EnableItems() {
    ui->tempUp->setEnabled(true);
    ui->tempDown->setEnabled(true);
    ui->windUp->setEnabled(true);
    ui->windDown->setEnabled(true);
    ui->modeButton->setEnabled(true);
    ui->working->setText(QString::fromWCharArray(L"等待送风"));
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
    ca.is_on = false;
    if (tempTimer->isActive())
        tempTimer->stop();
    if (notifyTimer->isActive())
        notifyTimer->stop();
    if (recoveryTimer->isActive())
        recoveryTimer->stop();
    delete tempTimer, notifyTimer, recoveryTimer;
    tempTimer = notifyTimer = recoveryTimer = NULL;
    this->close();
    emit toLogIn();
}

void Panel::TempUpClicked() {
    if (this->ca.expTemp != (int)TempRange::UPPER_BOUND) {
        ca.expTemp++;
        this->ui->expectedTemp->setText(QString::number(ca.expTemp) + " Centigrade");
        if (!sendTimer->isActive())
            sendTimer->start(SEND_WAIT_PERIOD);
    }
}

void Panel::TempDownClicked() {
    if (this->ca.expTemp != (int)TempRange::LOWER_BOUND) {
        ca.expTemp--;
        this->ui->expectedTemp->setText(QString::number(ca.expTemp) + " Centigrade");
        if (!sendTimer->isActive())
            sendTimer->start(SEND_WAIT_PERIOD);
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
        ReportState();
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
        ReportState();
    }
}

void Panel::ModeClicked() {
    ca.is_heat_mode = !ca.is_heat_mode;
    this->ui->mode->setText(QString::fromWCharArray(ca.is_heat_mode ? L"制热" : L"制冷"));
    ReportState();
}

void Panel::SwitchClicked() {
    if (!ca.is_on) {
        ReportState();
        ca.is_on = true;
        EnableItems();
    }
    else {
        ReportState();
        ca.is_on = false;
        ui->working->setText(QString::fromWCharArray(L"等待送风"));
        DisableItems();
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
        ui->working->setText(QString::fromWCharArray(L"等待送风"));
        recoveryTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)Speed::NORMAL_SPEED]);
    }
}

void Panel::ReportState() {
    json sendInfo = {
        {"op", REPORT_STATE},
        {"is_on", ca.is_on},
        {"is_heat_mode", ca.is_heat_mode},
        {"set_temp", ca.expTemp},
        {"real_temp", ca.temp},
        {"speed", TempInc[(int)ca.speed]}
    };
    json recvInfo = json::parse(_client->Send(sendInfo.dump()));
    if (!recvInfo.empty() && recvInfo.find("ret") != recvInfo.end()
            && recvInfo["ret"].get<int>() == REPLY_CON) {
        if (recvInfo.find("power") != recvInfo.end())
            ui->power->setText(QString::number(recvInfo["power"].get<double>()));
        if (recvInfo.find("money") != recvInfo.end())
            ui->cost->setText(QString::number(recvInfo["money"].get<double>()));
        if (recvInfo.find("is_valid") != recvInfo.end()) {
            if (recvInfo["is_valid"].get<bool>()) {
                if (!tempTimer->isActive()) {
                    tempTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)ca.speed]);
                    if (ui->working->text() != QString::fromWCharArray(L"送风中"))
                    ui->working->setText(QString::fromWCharArray(L"送风中"));
                }
            }
        }
    }
}

void Panel::RecoverTemp() {
    //温度恢复到可以忍受的最大范围
    if (ca.is_on && abs(ca.expTemp - ca.temp) == TEMP_BEAR_RANGE) {
        recoveryTimer->stop();
        tempTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)ca.speed]);
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

void Panel::ClusterSend() {
    ca.is_on = true;
    ReportState();
    this->sendTimer->stop();
}
