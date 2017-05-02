#ifndef PANEL_H
#define PANEL_H

#include <QWidget>
#include "client.h"
#include "conditionorattr.h"
#include "conditionorattr.h"

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
    void UpdateSetting();
    Ui::Panel *ui;
    Connor_Socket::Client *_client;
    int temp;
    int expTemp;
    Speed speed = Speed::FAST_SPEED;
    QTimer* timer;
    bool reachExpedtedTemp = true;
    int x = 0;

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

#endif // PANEL_H
