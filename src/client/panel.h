#ifndef PANEL_H
#define PANEL_H

#include <QWidget>
#include "client.h"
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
    Ui::Panel *ui;
    Connor_Socket::Client *_client;
    ConditionorAttr ca;
    QTimer* tempTimer, *notifyTimer;

signals:
    void toLogIn();

private slots:
    void Show(Connor_Socket::Client* c);
    void LogOutClicked();
    void TempUpClicked();
    void TempDownClicked();
    void WindUpClicked();
    void WindDownClicked();
    void UpdateTemp();
    void UpdateSetting();
};

#endif // PANEL_H
