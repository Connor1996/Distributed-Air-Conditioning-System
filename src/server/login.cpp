#include "login.h"
#include "ui_login.h"
#include <QMessageBox>
#include <QPainter>
#include <QBrush>

#include "src/include/json.hpp"
#include "src/protocol.h"

#include "management.h"

using Connor_Socket::Server;
using json = nlohmann::json;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    InitConnect();
    setWindowFlags(Qt::WindowCloseButtonHint); //只要关闭按钮

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
    std::string username = ui->usernameEdit->text().toStdString();
    std::string password = ui->passwordEdit->text().toStdString();
    if (username == "") {
        QMessageBox::information(this, "info", "username not filled");
    } else if (password == "") {
        QMessageBox::information(this, "info", "password not filled");
    } else if (username == "admin" && password == "admin"){
        this->close();
        Management window;
        window->show();
        //new management();
    } else {
        QMessageBox::information(this, "info", "wrong password");
        this->ui->passwordEdit->clear();
    }
}
