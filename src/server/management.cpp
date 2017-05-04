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
    InitWidget();
    InitConnect();

}

Management::~Management()
{
    delete ui;
    delete _server;
    //delete thread;
}

void Management::InitWidget() {
    //expTemp = temp = (int)TempRange::LOWER_BOUND + Random((int)TempRange::UPPER_BOUND - (int)TempRange::LOWER_BOUND);
    //QString t = QString::fromStdString(itos(temp) + " Centigrade");
//    ui->temperature->setText(t);
//    ui->expectedTemp->setText(t);
//    ui->WindSpeed->setText(QString::fromStdString(SpeedStr[(int)speed]));
    //tempChangeTimer = new QTimer();
}

void Management::InitConnect() {
//    QObject::connect(ui->logOutButton, SIGNAL(clicked(bool)), this, SLOT(LogOutClicked()));
//    QObject::connect(ui->tempUp, SIGNAL(clicked(bool)), this, SLOT(TempUpClicked()));
//    QObject::connect(ui->tempDown, SIGNAL(clicked(bool)), this, SLOT(TempDownClicked()));
//    QObject::connect(ui->windUp, SIGNAL(clicked(bool)), this, SLOT(WindUpClicked()));
//    QObject::connect(ui->windDown, SIGNAL(clicked(bool)), this, SLOT(WindDownClicked()));
//    QObject::connect(this->tempChangeTimer, SIGNAL(timeout()), this, SLOT(UpdateTemp()));
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
//        if (!tempChangeTimer->isActive())
//            tempChangeTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)speed]);
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
//        if (!tempChangeTimer->isActive())
//            tempChangeTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)speed]);
//        reachExpedtedTemp = false;
////        _client = new Client("");
////        json sendInfo = {{"op", REQ_RESUME}};
////        _client->Connect(sendInfo.dump());
////        delete _client;
//        UpdateSetting();
//    }
}




