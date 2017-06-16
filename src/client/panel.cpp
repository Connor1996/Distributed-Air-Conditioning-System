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
#include <QPixmap>
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

Panel::Panel(Connor_Socket::Client* client, bool is_heat_mode, int default_temp, QWidget *parent) :
    QWidget(parent),
    _client(client),
    _clientThread(new std::thread([this](){
        while(true) {
            try {
                ReceiveHandle(json::parse(_client->Receive()));
            } catch (std::exception e) {
                std::cout << "[ERROR] cannot connect to server" << std::endl;
                break;
            }
        }
    })),
    st({0, 0, false, 500}),
    ui(new Ui::Panel)
{
    ui->setupUi(this);

    ca.is_on = false;
    ca.is_heat_mode = is_heat_mode;
    ca.speed = Speed::NORMAL_SPEED;
    ca.expTemp = ca.temp = ca.original_temp = default_temp;
    ca.ntfy_frequence = NOTIFY_PERIOD;

    InitWidget();
    InitConnect();
}

Panel::~Panel()
{
    delete ui;
    delete tempTimer;
    delete notifyTimer;
    delete recoveryTimer;
    delete sendTimer;
    delete updateTimer;
    delete _client;
}

void Panel::InitWidget() {
    this->setWindowTitle(QString::fromWCharArray(L"从控机"));
    this->setWindowIcon(QIcon(":/server/fan"));
    this->setFixedHeight(320);

    clientRotationable = new RotationLabel(ROTAIONAL_SIZE, RotateRatio[1], NULL);
    ui->fanLayout->addWidget(clientRotationable);

    if (!ca.is_heat_mode)
        ui->mode_label->setPixmap(QPixmap(":/server/cold"));
    else
        ui->mode_label->setPixmap(QPixmap(":/server/warm"));

    ui->expLCD->display(ca.expTemp);

    tempTimer = new QTimer();
    notifyTimer = new QTimer();
    recoveryTimer = new QTimer();
    sendTimer = new QTimer();

    updateTimer = new QTimer();
    updateTimer->start(100);

    DisableItems();
}

void Panel::InitConnect() {
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
    QObject::connect(this->updateTimer, SIGNAL(timeout()), this, SLOT(Update()));
}


void Panel::DisableItems() {
    this->clientRotationable->Stop();
    ui->tempUp->setEnabled(false);
    ui->tempDown->setEnabled(false);
    ui->windUp->setEnabled(false);
    ui->windDown->setEnabled(false);
    ui->modeButton->setEnabled(false);
    ui->switchButton->setEnabled(true);
    ui->mode_label->setEnabled(false);
    ui->realLCD->setEnabled(false);
    ui->expLCD->setEnabled(false);
    clientRotationable->setEnabled(false);

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
    ui->mode_label->setEnabled(true);
    ui->realLCD->setEnabled(true);
    ui->expLCD->setEnabled(true);
    clientRotationable->setEnabled(true);

    ui->windSpeed->setText(QString::fromWCharArray(SpeedStr[(int)ca.speed].c_str()));
    ui->realLCD->display(ca.temp);
    ui->expLCD->display(ca.expTemp);
    ui->originalTemp->setText(QString::number(ca.original_temp) + QString::fromWCharArray(L" 度"));
    //notifyTimer->start(NOTIFY_PERIOD);
}


void Panel::TempUpClicked() {
    int upper_bound = ca.is_heat_mode ? (int)HeatRange::UPPER_BOUND : (int)ColdRange::UPPER_BOUND;
    if (this->ca.expTemp != upper_bound) {
        ca.expTemp++;
        ui->expLCD->display(ca.expTemp);
        if (!sendTimer->isActive())
            sendTimer->start(SEND_WAIT_PERIOD);
    }
}

void Panel::TempDownClicked() {
    int lower_bound = ca.is_heat_mode ? (int)HeatRange::LOWER_BOUND : (int)ColdRange::LOWER_BOUND;
    if (this->ca.expTemp != lower_bound) {
        ca.expTemp--;
        this->ui->expLCD->display(ca.expTemp);
        if (!sendTimer->isActive())
            sendTimer->start(SEND_WAIT_PERIOD);
    }
}

void Panel::WindUpClicked() {
    if (this->ca.speed != Speed::FAST_SPEED) {
        this->ca.speed = (Speed)(((int)this->ca.speed) + 1);
        this->ui->windSpeed->setText(QString::fromWCharArray(SpeedStr[(int)ca.speed].c_str()));
        this->clientRotationable->AdjustRotateRatio(RotateRatio[(int)ca.speed]);
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
        this->ui->windSpeed->setText(QString::fromWCharArray(SpeedStr[(int)ca.speed].c_str()));
        this->clientRotationable->AdjustRotateRatio(RotateRatio[(int)ca.speed]);
        if (tempTimer->isActive()) {
            tempTimer->stop();
            tempTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)ca.speed]);
        }
        ReportState();
    }
}

void Panel::ModeClicked() {
    ca.is_heat_mode = !ca.is_heat_mode;
    QPixmap picture = ca.is_heat_mode ? ":/server/warm" : ":/server/cold";
    this->ui->mode_label->setPixmap(picture);
    ReportState();
}

void Panel::SwitchClicked() {
    ca.is_on = !ca.is_on;
    ReportState();

    if (ca.is_on)
        EnableItems();
    else {
        DisableItems();
        recoveryTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)Speed::NORMAL_SPEED]);
    }
}

void Panel::AdjustTemp() {
    if (ca.expTemp > ca.temp) {
        ca.temp = ca.temp + 1 > ca.expTemp ? ca.expTemp : ca.temp + 1;
        this->ui->realLCD->display(ca.temp);
    }
    if (ca.expTemp < ca.temp) {
        ca.temp = ca.temp - 1 < ca.expTemp ? ca.expTemp : ca.temp - 1;
        this->ui->realLCD->display(ca.temp);
    }
    if (ca.expTemp == ca.temp) {
        if (tempTimer->isActive())
            tempTimer->stop();
        this->clientRotationable->Stop();
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

    try {
        _client->Send(sendInfo.dump());
    } catch (std::exception e) {
        QMessageBox::information(this, "info", "Can not connect server");
        DisableItems();
        this->close();
        return;
    }
}

void Panel::Update()
{
    ui->power->setText(QString::number(st.power) + QString::fromWCharArray(L" 瓦特"));
    ui->cost->setText(QString::number(st.cost) + QString::fromWCharArray(L" 元"));
    ca.ntfy_frequence = std::max(100, ca.ntfy_frequence);

    if (ca.is_on && !notifyTimer->isActive()) {
        ca.ntfy_frequence = st.frequence;
        notifyTimer->start(ca.ntfy_frequence);
    }
    if (notifyTimer->isActive() && ca.ntfy_frequence != st.frequence) {
        ca.ntfy_frequence = st.frequence;
        notifyTimer->stop();
        notifyTimer->start(ca.ntfy_frequence);
    }

    if (st.is_valid) {
        if (!tempTimer->isActive()) {
            tempTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)ca.speed]);
        }
        if (!this->clientRotationable->isActive())
            this->clientRotationable->Start();
        if (recoveryTimer->isActive()) {
            std::cout << "stop recovery timer" << std::endl;
            recoveryTimer->stop();
        }
    }
    else {
        if (tempTimer->isActive()) {
            tempTimer->stop();
        }
        if (this->clientRotationable->isActive())
            this->clientRotationable->Stop();
        if (!recoveryTimer->isActive())
            recoveryTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)Speed::NORMAL_SPEED]);
    }
}

void Panel::ReceiveHandle(json recvInfo) {
    if (!recvInfo.empty() && recvInfo["ret"].get<int>() == REPLY_CON) {
        st.power = recvInfo["power"].get<double>();
        st.cost = recvInfo["cost"].get<double>();
        st.frequence = recvInfo["frequence"].get<int>();
        st.is_valid = recvInfo["is_valid"].get<bool>();
    }



//    if (!recvInfo.empty() && recvInfo.find("ret") != recvInfo.end()
//            && recvInfo["ret"].get<int>() == REPLY_CON) {
//        if (recvInfo.find("power") != recvInfo.end())
//            ui->power->setText(QString::number()) + QString::fromWCharArray(L" 瓦特"));
//        if (recvInfo.find("cost") != recvInfo.end())
//            ui->cost->setText(QString::number() + QString::fromWCharArray(L" 元"));
//        if (recvInfo.find("frequence") != recvInfo.end()) {
//            if (ca.is_on && !notifyTimer->isActive()) {
//                ca.ntfy_frequence = recvInfo["frequence"].get<int>();
//                notifyTimer->start(ca.ntfy_frequence);
//            }
//            if (notifyTimer->isActive() && ca.ntfy_frequence != recvInfo["frequence"].get<int>()) {
//                ca.ntfy_frequence = recvInfo["frequence"].get<int>();
//                notifyTimer->stop();
//                notifyTimer->start(ca.ntfy_frequence);
//            }
//        }
//        if (recvInfo.find("is_valid") != recvInfo.end()) {
//            if (recvInfo["is_valid"].get<bool>()) {
//                if (!tempTimer->isActive()) {
//                    tempTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)ca.speed]);
//                }
//                if (!this->clientRotationable->isActive())
//                    this->clientRotationable->Start();
//                if (recoveryTimer->isActive()) {
//                    std::cout << "stop recovery timer" << std::endl;
//                    recoveryTimer->stop();
//                }
//            }
//            else {
//                if (tempTimer->isActive()) {
//                    tempTimer->stop();
//                }
//                if (this->clientRotationable->isActive())
//                    this->clientRotationable->Stop();
//                if (!recoveryTimer->isActive())
//                    recoveryTimer->start(TEMP_CHANGE_CIRCUIT / TempInc[(int)Speed::NORMAL_SPEED]);
//            }
//        }
//    }
}

void Panel::RecoverTemp() {
    //温度恢复到可以忍受的最大范围
    if (ca.is_on && abs(ca.expTemp - ca.temp) == TEMP_BEAR_RANGE) {
        ReportState();
    }
    if (recoveryTimer->isActive() && ca.temp == ca.original_temp) {
        recoveryTimer->stop();
    }
    if (recoveryTimer->isActive() && ca.temp < ca.original_temp) {
        ca.temp = ca.temp + 1 > ca.original_temp ? ca.original_temp : ca.temp + 1;
        this->ui->realLCD->display(ca.temp);
    }
    if (recoveryTimer->isActive() && ca.temp > ca.original_temp) {
        ca.temp = ca.temp - 1 < ca.original_temp ? ca.original_temp : ca.temp - 1;
        this->ui->realLCD->display(ca.temp);
    }
}

void Panel::ClusterSend() {
    ca.is_on = true;
    ReportState();
    this->sendTimer->stop();
}
