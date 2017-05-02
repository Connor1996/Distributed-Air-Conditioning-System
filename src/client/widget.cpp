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
        _client = new Client(room);
        int room_id = atoi(room.c_str());
        json sendInfo = {
            {"op", LOG_IN_USER},
            {"room_id", room_id},
            {"user_id", user_id}
        };
        //json receiveInfo = json::parse(_client->Connect(sendInfo.dump()));
        json receiveInfo = {{"ret", LOG_IN_SUCC}};
        if (receiveInfo["ret"].get<int>() == LOG_IN_FAIL)
            QMessageBox::information(this, "info", "Log in failed");
        else {
            this->close();
            this->ui->IDEdit->clear();
            this->ui->RoomEdit->clear();
            this->ui->RoomEdit->setFocus();
            emit toPanel();
        }
    }
//    auto username = ui->usernameEdit->text().toStdString();
//    auto password = ui->passwordEdit->text().toStdString();

//    try
//    {
//        // 发送登陆请求
//       _client = new Client(username);
//       json sendInfo = {
//           {"define", LOG_IN},
//           {"username", username},
//           {"password", password}
//       };
//       json receiveInfo = json::parse(_client->Connect(sendInfo.dump()));

//       // 检查是否登陆成功
//       if (receiveInfo["define"].get<int>() == LOG_IN_FAIL_WP)
//       {
//           _client->Close();
//           delete _client;
//           QMessageBox::information(this, "Error", QString::fromLocal8Bit("登陆失败,用户名或密码错误"));
//       }
//       else if (receiveInfo["define"].get<int>() == LOG_IN_FAIL_AO)
//       {
//           _client->Close();
//           delete _client;
//           QMessageBox::information(this, "Error", QString::fromLocal8Bit("登陆失败，该用户已经在线"));
//       }
//       else if (receiveInfo["define"].get<int>() == LOG_IN_SUCCESS)
//       {
//           // 进入主界面

//           try
//           {
//               this->close();
//               StackWidget *stack = new StackWidget(_client);
//                stack->show();
//           }
//           catch (std::exception e)
//           {
//               QMessageBox::information(this, "Error", QString::fromLocal8Bit("与服务器断开连接"));
//           }
//       }
//       else if (receiveInfo["define"].get<int>() == SERVER_ERROR)
//       {
//           throw std::runtime_error("Server occurs fatal error");
//       }
//       else
//       {
//           throw std::runtime_error("Wrong return value for request");
//       }
//    }
//    catch (std::exception e)
//    {
//        _client->Close();
//        delete _client;
//        QMessageBox::information(this, "Error", QString(e.what()));
//    }
}
