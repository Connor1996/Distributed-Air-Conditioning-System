#include "login.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget* w(new Widget());
    //Panel* p(new Panel());
    w->show();

    //QObject::connect(w, SIGNAL(toPanel()), p, SLOT(Show()));
    //QObject::connect(p, SIGNAL(toLogIn()), w, SLOT(show()));

    return a.exec();
}
