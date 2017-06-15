
#include "widget.h"
#include "panel.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget* w(new Widget());
    w->show();


    return a.exec();
}
