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

    Ui::Management *ui;
    Connor_Socket::Server *_server;

};

#endif // MANAGEMENT_H
