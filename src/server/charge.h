#ifndef CHARGE_H
#define CHARGE_H

#include <QWidget>

namespace Ui {
class charge;
}

class charge : public QWidget
{
    Q_OBJECT

public:
    explicit charge(QString room_id,QString user_id,QString total_time,QString total_money,QWidget *parent = 0);
    ~charge();

private slots:
    void on_make_sure_clicked();

private:
    Ui::charge *ui;
};

#endif // CHARGE_H
