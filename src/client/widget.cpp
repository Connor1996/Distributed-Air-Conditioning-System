#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QPainter>
#include <QBrush>

#include "src/include/json.hpp"
#include "src/protocol.h"

using Connor_Socket::Client;
using json = nlohmann::json;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    InitConnect();
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    //setWindowFlags(Qt::WindowCloseButtonHint); //只要关闭按钮

    setFixedSize(1024, 605);
    // 设置背景
    setAutoFillBackground(true);
}

void Widget::InitConnect()
{
    // 触发登陆
    connect(ui->loginButton, SIGNAL(clicked()), this, SLOT(Login()));
    connect(ui->cancelButtion, SIGNAL(clicked(bool)), this, SLOT(Cancel()));
}

Widget::~Widget()
{
    delete ui;
    if (_client != nullptr) {
        delete _client;
        _client = nullptr;
    }
}


void Widget::Cancel() {
    this->close();
}

void Widget::Show() {
    if (_client != nullptr) {
        delete _client;
        std::cout << "delete" << std::endl;
        _client = nullptr;
        std::cout << "nullptr" << std::endl;
    }
    this->show();
    std::cout << "show" << std::endl;
}

void Widget::Login()
{
    std::string room = ui->RoomEdit->text().toStdString();
    std::string user_id = ui->IDEdit->text().toStdString();
    if (room == "") {
        QMessageBox::information(this, "info", "Room not filled");
    }
    else if (user_id == "") {
        QMessageBox::information(this, "info", "ID not filled");
    }
    else {
        _client = new Client(room);
        int room_id = atoi(room.c_str());
        json sendInfo = {
            {"op", LOG_IN_USER},
            {"room_id", room_id},
            {"user_id", user_id}
        };
        json receiveInfo;
        try {
            receiveInfo = json::parse(_client->Connect(sendInfo.dump()));
        }
        catch (std::exception e) {
            delete _client;
            _client = nullptr;
            QMessageBox::information(this, "info", "Can not connect server");
            return;
        }
        if (receiveInfo["ret"].get<int>() == LOG_IN_FAIL) {
            QMessageBox::information(this, "info", "Room No. or ID incorrect");
        }
        else {
            this->close();
            this->ui->IDEdit->clear();
            this->ui->RoomEdit->clear();
            this->ui->RoomEdit->setFocus();
            std::cout << "IDEdit = " << ui->IDEdit->text().toStdString() << " RoomEdit=" << ui->RoomEdit->text().toStdString() << std::endl;
            emit toPanel(_client);
        }
    }
}
