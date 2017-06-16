#ifndef WIDGET_H
#define WIDGET_H

#include "server.h"

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QtCharts>


using Server = Connor_Socket::Server;
using namespace QtCharts;

namespace Ui {
class Form;
}

class Form : public QWidget
{

    Q_OBJECT

public:
    explicit Form(Server *server, QWidget *parent = 0);
    Server *_server;
    QGraphicsScene *piescene;
    QGraphicsView *pieview;
    QPieSeries *pieSeries;
    QChart *pieChart;
    QStandardItemModel *model3;
    QGraphicsScene *linescene;
    QGraphicsView *lineview;
    QLineSeries *lineseries;
    QChart *lineChart;
    QGraphicsScene *monthlinescene;
    QGraphicsView *monthlineview;
    QLineSeries *monthlineseries;
    QChart *monthlineChart;

    void barinit(QWidget *w, int *s);
    void tableinit(QTableView *t, std::vector<Request> data);
    double getcost(int r);
    ~Form();

private:
    Ui::Form *ui;

    void InitConnect();
};

#endif // WIDGET_H
