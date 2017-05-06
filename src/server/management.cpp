#define _CRT_RAND_S
#include <stdlib.h>

#include "management.h"
#include "ui_management.h"

#include "src/include/json.hpp"
#include "src/protocol.h"

#include <QMessageBox>
#include <QTimer>

using Connor_Socket::Server;
using json = nlohmann::json;


Management::Management(QWidget *parent) :
    QWidget(parent),
    _thread(new std::thread([this](){
        _server = new Server();
        _server->Start();
    })),
    ui(new Ui::Management)
{
    ui->setupUi(this);
    InitWidget();
    InitConnect();

}

Management::~Management()
{
    delete ui;
    delete _server;
    delete _thread;
}

#define DEFAULT_TEMP 18
void Management::InitWidget() {
    ui->tempNumber->display(DEFAULT_TEMP);
}
#define MIN_TEMP 16
#define MAX_TEMP 30

void Management::InitConnect() {
    connect(ui->tempDownButton, &QPushButton::clicked, [this](){
        ui->tempNumber->display(max(MIN_TEMP, ui->tempNumber->intValue() - 1));
    });

    connect(ui->tempUpButton, &QPushButton::clicked, [this](){
        ui->tempNumber->display(min(MAX_TEMP, ui->tempNumber->intValue() + 1));
    });

    connect(ui->checkInButton, &QPushButton::clicked, [this](){
        int roomId;
        try {
            roomId = ui->roomId->currentText().toInt();
        } catch (std::exception) {
            QMessageBox::information(this, "info", "please input digital roomid");
        }

        std::string userId = ui->userIdEdit->text().toStdString();
        if (_server->CheckIn(roomId, userId))
            QMessageBox::information(this, "info", "check in successful");
        else
            QMessageBox::information(this, "info", "already check in");

        ui->userIdEdit->clear();
    });

    connect(ui->checkOutButton, &QPushButton::clicked, [this](){
        int roomId = ui->roomId->currentText().toInt();
        if (!_server->CheckOut(roomId))
            QMessageBox::information(this, "info", "this roomId is not checked in");
        else
            QMessageBox::information(this, "info", "check out successful");

        ui->userIdEdit->clear();
    });
}







