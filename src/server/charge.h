#ifndef CHARGE_H
#define CHARGE_H

#include <QWidget>

namespace Ui {
class Charge;
}

class Charge : public QWidget
{
    Q_OBJECT

public:
    explicit Charge(QString room_id,QString user_id,QString total_time,QString total_money,QWidget *parent = 0);
    ~Charge();

private slots:
    void on_make_sure_clicked();

private:
    Ui::Charge *ui;
};

#endif // CHARGE_H
