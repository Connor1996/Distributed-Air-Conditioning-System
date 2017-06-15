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
    ui(new Ui::Widget),
    _client(nullptr)
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
    delete _client;
}


void Widget::Cancel() {
    this->close();
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
        std::string ipaddr;
        int port;
        if (ui->ipLineEdit->text().isEmpty() && ui->portLineEdit->text().isEmpty()) {
               ipaddr = DEFAULT_SERVER_IP;
               port = DEFAULT_SERVER_PORT;
        }
        else if (ui->ipLineEdit->text().isEmpty() && !ui->portLineEdit->text().isEmpty()
                 || !ui->ipLineEdit->text().isEmpty() && ui->portLineEdit->text().isEmpty()) {
            QMessageBox::information(this, "info", "Specify server's ip and port OR use default");
        }
        else {
            ipaddr = this->ui->ipLineEdit->text().toStdString();
            port = this->ui->portLineEdit->text().toInt();
        }
        _client = new Client(room, ipaddr, port);

        int room_id = atoi(room.c_str());
        json sendInfo = {
            {"op", LOG_IN_USER},
            {"room_id", room_id},
            {"user_id", user_id}
        };
        json receiveInfo;

        try {
            receiveInfo = json::parse(_client->Connect(sendInfo.dump()));
        } catch (std::exception e) {
            std::cout << e.what() << std::endl;;
            delete _client;
            QMessageBox::information(this, "info", "Can not connect server");
            return;
        }

        if (receiveInfo["ret"].get<int>() == LOG_IN_FAIL) {
            QMessageBox::information(this, "info", "Room No. or ID incorrect");
        }
        else {
            this->ui->IDEdit->clear();
            this->ui->RoomEdit->clear();
            this->ui->RoomEdit->setFocus();
            this->close();
            Panel* p(new Panel(_client,
                               receiveInfo["is_heat_mode"].get<bool>(),
                               receiveInfo["default"].get<int>()
                    ));
            p->show();
        }
    }
}
