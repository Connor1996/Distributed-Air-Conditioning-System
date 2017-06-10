#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QtCharts>

using namespace QtCharts;

namespace Ui {
class Widget;
}

class Form : public QWidget
{

    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);

    void barinit(QWidget *w, int *s);
    void tableinit(QTableView *t, QStringList data);
    ~Form();

private:
    Ui::Widget *ui;

    void InitConnect();
};

#endif // WIDGET_H
