#include "charge.h"
#include "ui_charge.h"

#include<QMessageBox>

Charge::Charge(QString room_id, QString user_id, QString total_time,
               QString total_money, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Charge)
{
    ui->setupUi(this);
    ui->room_id->setText(room_id);
    ui->user_id->setText(user_id);
    ui->total_time->setText(total_time);
    ui->total_money->setText(total_money);
}

Charge::~Charge()
{
    delete ui;
}

void Charge::on_make_sure_clicked()
{
    QMessageBox::information(this, "info", "check out successful");
    this->close();
}
