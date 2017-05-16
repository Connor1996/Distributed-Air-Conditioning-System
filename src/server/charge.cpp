#include "charge.h"
#include "ui_charge.h"

#include<QMessageBox>

charge::charge(QString room_id,QString user_id,QString total_time,QString total_money,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::charge)
{
    ui->setupUi(this);
    ui->room_id->setText(room_id);
    ui->user_id->setText(user_id);
    ui->total_time->setText(total_time);
    ui->total_money->setText(total_money);
}

charge::~charge()
{
    delete ui;
}

void charge::on_make_sure_clicked()
{
    QMessageBox::information(this, "info", "check out successful");
    this->close();
}
