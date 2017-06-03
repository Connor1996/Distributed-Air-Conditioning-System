#include "login.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setFont(QFont("Microsoft Yahei", 10));
    Widget* w(new Widget());

    w->show();

    return a.exec();
}
