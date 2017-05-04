#include "widget.h"
#include "panel.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget* w(new Widget());
    Panel* p(new Panel());
    w->show();

    QObject::connect(w, SIGNAL(toPanel(Connor_Socket::Client*)), p, SLOT(Show(Connor_Socket::Client*)));
    QObject::connect(p, SIGNAL(toLogIn()), w, SLOT(Show()));

    return a.exec();
}
