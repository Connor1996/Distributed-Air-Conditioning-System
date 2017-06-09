#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include "server.h"
#include "charge.h"

#include <QWidget>
#include <QLabel>
#include <QLCDNumber>

#include <unordered_map>
#include "src/rotationlabel.h"

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

    // server socket对象
    Connor_Socket::Server *_server;
    std::thread *_serverThread;
    QTimer *_updateTimer;

    struct RoomLabels {
        QLabel *picLabel;
        RotationLabel *fanLabel;
        QLCDNumber *setTemp;
        QLCDNumber *realTemp;
    };

    std::unordered_map<int, struct RoomLabels> _labels;
    std::list<int> _roomIds;

    Charge *_charge;

};

#endif // MANAGEMENT_H
