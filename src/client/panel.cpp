#define _CRT_RAND_S
#include<stdlib.h>

#include "panel.h"
#include "ui_panel.h"

#include <sstream>
#include "src/include/json.hpp"
#include "src/protocol.h"
#include "conditionorattr.h"

#include <QTimer>

using Connor_Socket::Client;
using json = nlohmann::json;

unsigned int Random(int max)
{
   errno_t err;
   unsigned int number;
   err = rand_s(&number);
   if(err != 0)
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
    delete timer;
}

void Panel::Show() {
    InitWidget();
    InitConnect();
    this->show();
    UpdateSetting();
}

void Panel::InitWidget() {
    expTemp = temp = (int)TempRange::LOWER_BOUND + Random((int)TempRange::UPPER_BOUND - (int)TempRange::LOWER_BOUND);
    QString t = QString::fromStdString(itos(temp) + " Centigrade");
    ui->temperature->setText(t);
    ui->expectedTemp->setText(t);
    ui->WindSpeed->setText(QString::fromStdString(SpeedStr[(int)speed]));
    timer = new QTimer();
}

void Panel::InitConnect() {
    QObject::connect(ui->logOutButton, SIGNAL(clicked(bool)), this, SLOT(LogOutClicked()));
    QObject::connect(ui->tempUp, SIGNAL(clicked(bool)), this, SLOT(TempUpClicked()));
    QObject::connect(ui->tempDown, SIGNAL(clicked(bool)), this, SLOT(TempDownClicked()));
    QObject::connect(ui->windUp, SIGNAL(clicked(bool)), this, SLOT(WindUpClicked()));
    QObject::connect(ui->windDown, SIGNAL(clicked(bool)), this, SLOT(WindDownClicked()));
    QObject::connect(this->timer, SIGNAL(timeout()), this, SLOT(UpdateTemp()));
}

void Panel::UpdateSetting() {
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

void Panel::LogOutClicked() {
    //json sendInfo = {{"op", REQ_STOP}};
    //_client = new Client("");
    //_client->Connect(sendInfo.dump());
    this->close();
    emit toLogIn();
}

void Panel::TempUpClicked() {
    if (this->expTemp != (int)TempRange::UPPER_BOUND) {
        expTemp++;
        this->ui->expectedTemp->setText(QString::fromStdString(itos(expTemp) + " Centigrade"));
        if (!timer->isActive())
            timer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)speed]);
        reachExpedtedTemp = false;
//        _client = new Client("");
//        json sendInfo = {{"op", REQ_RESUME}};
//        _client->Connect(sendInfo.dump());
//        delete _client;
        UpdateSetting();
    }
}

void Panel::TempDownClicked() {
    if (this->expTemp != (int)TempRange::LOWER_BOUND) {
        expTemp--;
        this->ui->expectedTemp->setText(QString::fromStdString(itos(expTemp) + " Centigrade"));
        if (!timer->isActive())
            timer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)speed]);
        reachExpedtedTemp = false;
//        _client = new Client("");
//        json sendInfo = {{"op", REQ_RESUME}};
//        _client->Connect(sendInfo.dump());
//        delete _client;
        UpdateSetting();
    }
}

void Panel::WindUpClicked() {
    if (this->speed != Speed::FAST_SPEED) {
        this->speed = (Speed)(((int)this->speed) + 1);
        this->ui->WindSpeed->setText(QString::fromStdString(SpeedStr[(int)speed]));
        if (timer->isActive()) {
            timer->stop();
            timer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)speed]);
        }
        UpdateSetting();
    }
}

void Panel::WindDownClicked() {
    if (this->speed != Speed::SLOW_SPEED) {
        this->speed = (Speed)(((int)this->speed) - 1);
        this->ui->WindSpeed->setText(QString::fromStdString(SpeedStr[(int)speed]));
        if (timer->isActive()) {
            timer->stop();
            timer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)speed]);
        }
        UpdateSetting();
    }
}

void Panel::UpdateTemp() {
    std::cout << "timeout " << x++ << std::endl;
    if (expTemp > temp) {
        temp = temp + 1 > expTemp ? expTemp : temp + 1;
        this->ui->temperature->setText(QString::fromStdString(itos(temp) + " Centigrade"));
    }
    if (expTemp < temp) {
        temp = temp - 1 < expTemp ? expTemp : temp - 1;
        this->ui->temperature->setText(QString::fromStdString(itos(temp) + " Centigrade"));
    }
    if (!reachExpedtedTemp && expTemp == temp) {
        reachExpedtedTemp = true;
        timer->stop();
        //_client = new Client("");
        //json sendInfo = {{"op", REQ_STOP}};
        //_client->Connect(sendInfo.dump());
        //delete _client;
    }
}
