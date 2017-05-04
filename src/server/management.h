#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include <QWidget>
#include "server.h"

namespace Ui {
class Management;
}

class Management : public QWidget
{
    Q_OBJECT

public:
    explicit Management(QWidget *parent = 0);
    ~Management();

private:
    void InitWidget();
    void InitConnect();
    void UpdateSetting();

    Ui::Management *ui;
    Connor_Socket::Server *_server;

signals:
    void toLogIn();

private slots:
    void Show();
    void LogOutClicked();
    void TempUpClicked();
    void TempDownClicked();
    void WindUpClicked();
    void WindDownClicked();
    void UpdateTemp();
};

#endif // MANAGEMENT_H
