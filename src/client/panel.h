#ifndef PANEL_H
#define PANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include "client.h"
#include "conditionorattr.h"
#include "src/rotationlabel.h"
#include "src/include/json.hpp"

using json = nlohmann::json;

const int NOTIFY_PERIOD = 1000;
const int SEND_WAIT_PERIOD = 1000;
const int ROTAIONAL_SIZE = 256;

namespace Ui {
class Panel;
}

class Panel : public QWidget
{
    Q_OBJECT

public:
    explicit Panel(Connor_Socket::Client *, bool, int, QWidget *parent = 0);
    ~Panel();

private:
    void InitWidget();
    void InitConnect();
    void DisableItems();
    void EnableItems();
    void ReceiveHandle(json recvInfo);

    Ui::Panel *ui;
    Connor_Socket::Client *_client;

    ConditionorAttr ca;
    QTimer* tempTimer, *notifyTimer, *recoveryTimer, *sendTimer;
    QTimer* updateTimer;
    RotationLabel* clientRotationable;
    QVBoxLayout* vwrVLayout;

    std::thread *_clientThread;

    struct State{
        double power;
        double cost;
        bool is_valid;
        int frequence;
    } st;

private slots:
    void TempUpClicked();
    void TempDownClicked();
    void WindUpClicked();
    void WindDownClicked();
    void ModeClicked();
    void SwitchClicked();
    void AdjustTemp();
    void ReportState();
    void RecoverTemp();
    void ClusterSend();

    void Update();
};

#endif // PANEL_H
