#ifndef CHARGE_H
#define CHARGE_H

#include <QDialog>
#include <QMessageBox>
#include "ui_charge.h"

namespace Ui {
class Charge;
}

class Charge : public QDialog
{
    Q_OBJECT

public:
    explicit Charge(QString room_id, QString user_id,
                    QString total_time, QString total_money, QDialog *parent = 0) :
        QDialog(parent),
        ui(new Ui::Charge)
    {
        ui->setupUi(this);
        ui->room_id->setText(room_id);
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
