#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "server.h"
#include "management.h"

namespace Ui {
class Widget;
}

// 登陆界面
class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    // 登陆
    void Login();

private:
    // 初始化UI界面
    void InitUi();

    // 初始化信号槽
    void InitConnect();

    // 该widget的ui界面指针
    Ui::Widget *ui;

    Management *window;
};

#endif // WIDGET_H
