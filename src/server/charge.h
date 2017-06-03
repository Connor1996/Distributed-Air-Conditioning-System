#ifndef CHARGE_H
#define CHARGE_H

#include <QWidget>
#include <QMessageBox>
#include "ui_charge.h"

namespace Ui {
class Charge;
}

class Charge : public QWidget
{
    Q_OBJECT

public:
    explicit Charge(QString room_id, QString user_id,
                    QString total_time, QString total_money, QWidget *parent = 0) :
        QWidget(parent),
        ui(new Ui::Charge)
    {
        ui->setupUi(this);
        ui->room_id->setText(room_id);
        ui->user_id->setText(user_id);
        ui->total_time->setText(total_time);
        ui->total_money->setText(total_money);
    }


    ~Charge() { delete ui; }

private slots:
    void on_make_sure_clicked() {
        this->close();
    }

private:
    Ui::Charge *ui;
};

#endif // CHARGE_H
