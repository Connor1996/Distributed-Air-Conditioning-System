#ifndef PANEL_H
#define PANEL_H

#include <QWidget>
#include "client.h"
#include "conditionorattr.h"

const int NOTIFY_PERIOD = 1000;
const int SEND_WAIT_PERIOD = 1000;

namespace Ui {
class Panel;
}

class Panel : public QWidget
{
    Q_OBJECT

public:
    explicit Panel(QWidget *parent = 0);
    ~Panel();

private:
    void InitWidget();
    void InitConnect();
    bool UpdateRequest();
    void DisableItems();
    void EnableItems();
    Ui::Panel *ui;
    Connor_Socket::Client *_client;
    ConditionorAttr ca;
    QTimer* tempTimer, *notifyTimer, *recoveryTimer, *sendTimer;

signals:
    void toLogIn();

private slots:
    void Show(Connor_Socket::Client* c);
    void LogOutClicked();
    void TempUpClicked();
    void TempDownClicked();
    void WindUpClicked();
    void WindDownClicked();
    void ModeClicked();
    void SwitchClicked();
    void AdjustTemp();
    void ReportState();
    void RecoverTemp();
    void SendNow();
};

#endif // PANEL_H
