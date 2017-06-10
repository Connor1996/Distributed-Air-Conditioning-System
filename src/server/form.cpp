#include "form.h"
#include "ui_form.h"

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    InitConnect();

    ui->tabWidget->setCurrentIndex(0);
    ui->toolBox->setCurrentIndex(2);
    ui->toolBox_2->setCurrentIndex(2);
    ui->toolBox_3->setCurrentIndex(2);

    int bar_room[8];
    for(int i=0; i<8; i++){
        bar_room[i] = i%4;
    }
    barinit(ui->page_1, bar_room);

    QStringList combo_room;
    combo_room << "room" << "411" << "412" << "413" << "414" << "415" <<
                  "416" << "417" << "418";
    ui->comboBox->addItems(combo_room);
    ui->comboBox_2->addItems(combo_room);
    ui->comboBox_3->addItems(combo_room);


    //page1 饼图
    QGraphicsScene *piescene = new QGraphicsScene();
    QGraphicsView *pieview = new QGraphicsView(ui->page_3);
    pieview->setScene(piescene);
    pieview->setRenderHint(QPainter::Antialiasing);
    piescene->setBackgroundBrush(QBrush(QColor(240, 240, 240)));
    pieview->setSceneRect(10, 10, 400, 300);
    pieview->setSizeIncrement(400,300);
    pieview->resizeAnchor();

    QPieSeries *pieSeries = new QPieSeries();
    pieSeries->append("411", 12);
    pieSeries->append("412", 12);
    pieSeries->append("413", 12);
    pieSeries->append("414", 12);
    pieSeries->append("415", 12);
    pieSeries->append("416", 12);
    pieSeries->append("417", 12);
    pieSeries->append("418", 16);

    QChart *pieChart = new QChart();
    pieChart->addSeries(pieSeries);  // 将 series 添加至图表中
    pieChart->legend()->setAlignment(Qt::AlignRight);  // 设置图例靠右显示
    pieChart->setTitle("Simple pie chart");
    pieChart->setGeometry(10, 10, 400, 300);
    piescene->addItem(pieChart);
    //room_cost table
    QStandardItemModel *model3 = new QStandardItemModel();

    model3->setColumnCount(2);
    model3->setHeaderData(0,Qt::Horizontal,"roomid");
    model3->setHeaderData(1,Qt::Horizontal,"cost");

    ui->costtable->setModel(model3);
    ui->costtable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    ui->costtable->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);
    ui->costtable->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Fixed);

    //page2 折线图
    QGraphicsScene *linescene = new QGraphicsScene();
    QGraphicsView *lineview = new QGraphicsView(ui->page_6);
    lineview->setScene(linescene);
    lineview->setRenderHint(QPainter::Antialiasing);
    linescene->setBackgroundBrush(QBrush(QColor(240, 240, 240)));
    lineview->setSceneRect(10, 10,560, 300);
    lineview->setSizeIncrement(560,300);
    lineview->resizeAnchor();

    QCategoryAxis *axisX = new QCategoryAxis();
    QCategoryAxis *axisY = new QCategoryAxis();
    axisY->setRange(0,6);
    axisX->setRange(1,7);



    // 构建 series，作为图表的数据源
    QLineSeries *lineseries = new QLineSeries();
    for(int i=0; i<7; i++){
        lineseries->append(i+1, i%3);
    }
    QChart *lineChart = new QChart();
    lineChart->addSeries(lineseries);  // 将 series 添加至图表中
    lineChart->setAxisX(axisX, lineseries);
    lineChart->setAxisY(axisY, lineseries);

    lineChart->legend()->hide();
    lineChart->createDefaultAxes();
    lineChart->setTitle("Simple line chart");
    lineChart->setGeometry(10, 10, 560, 300);
    linescene->addItem(lineChart);

    //page3 折线图
    QGraphicsScene *monthlinescene = new QGraphicsScene();
    QGraphicsView *monthlineview = new QGraphicsView(ui->page_9);
    monthlineview->setScene(monthlinescene);
    monthlineview->setRenderHint(QPainter::Antialiasing);
    monthlinescene->setBackgroundBrush(QBrush(QColor(240, 240, 240)));
    monthlineview->setSceneRect(10, 10,560, 300);
    monthlineview->setSizeIncrement(560,300);
    monthlineview->resizeAnchor();

    QCategoryAxis *monthaxisX = new QCategoryAxis();
    QCategoryAxis *monthaxisY = new QCategoryAxis();
    monthaxisY->setRange(0,6);
    monthaxisX->setRange(0,30);
    monthaxisX->setLabelFormat("%d");
    // 构建 series，作为图表的数据源
    QLineSeries *monthlineseries = new QLineSeries();
    for(int i=0; i<31; i++){
        monthlineseries->append(i, i%6);
    }

    QChart *monthlineChart = new QChart();
    monthlineChart->addSeries(monthlineseries);  // 将 series 添加至图表中
    monthlineChart->setAxisX(monthaxisX, monthlineseries);
    monthlineChart->setAxisY(monthaxisY, monthlineseries);

    monthlineChart->legend()->hide();
    monthlineChart->createDefaultAxes();
    monthlineChart->setTitle("Simple line chart");
    monthlineChart->setGeometry(10, 10, 560, 300);
    monthlinescene->addItem(monthlineChart);
}

Form::~Form()
{
    delete ui;
}

void Form::InitConnect() {
    connect(ui->comboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged)
            , [this](QString index){
                    if(index != "room") {
                        QStringList data;
                        tableinit(ui->tableView, data);
                    }
    });
}

void Form::barinit(QWidget *w, int *s){
    //柱状图
    QGraphicsScene *barscene = new QGraphicsScene();
    QGraphicsView *barview = new QGraphicsView(w);
    barview->setScene(barscene);
    barview->setRenderHint(QPainter::Antialiasing);
    barscene->setBackgroundBrush(QBrush(QColor(240, 240, 240)));
    barview->setSceneRect(20, 10, 745, 325);
    barview->setSizeIncrement(w->size());
    barview->resizeAnchor();

    auto setroom = new QBarSet("on/off");
    setroom->setBrush(QBrush(Qt::darkBlue));
    *setroom << s[0] << s[1] << s[2] << s[3] << s[4] << s[5] << s[6] << s[7];

    QBarSeries *barseries = new QBarSeries();
    barseries->append(setroom);

    QChart *barchart = new QChart();
    barchart->legend()->hide();
    barchart->addSeries(barseries);
    barchart->createDefaultAxes();
    barchart->setTitle("Simple barchart example");
    barchart->setGeometry(20,8,745,330);

    QStringList categories;
    categories << "411" << "412" << "413" << "414" << "415" <<
                  "416" << "417" << "418";
    auto axisX = new QBarCategoryAxis();
    axisX->append(categories);
    barchart->setAxisX(axisX, barseries);
    barchart->legend()->setVisible(true);
    barchart->legend()->setAlignment(Qt::AlignBottom);

    barscene->addItem(barchart);
}

void Form::tableinit(QTableView *t, QStringList data){
    //tableview
    QStandardItemModel *model = new QStandardItemModel();

    model->setColumnCount(6);
/*
    model->setHeaderData(0,Qt::Horizontal,QString::fromUtf8("温控开始时间"));
    model->setHeaderData(1,Qt::Horizontal,QString::fromUtf8("温控结束时间"));
    model->setHeaderData(2,Qt::Horizontal,QString::fromUtf8("温度请求开始温度"));
    model->setHeaderData(3,Qt::Horizontal,QString::fromUtf8("温度请求结束温度"));
    model->setHeaderData(4,Qt::Horizontal,QString::fromUtf8("温控风量大小"));
    model->setHeaderData(5,Qt::Horizontal,QString::fromUtf8("本次温控请求所需费用"));
*/
    model->setHeaderData(0,Qt::Horizontal,"starttime");
    model->setHeaderData(1,Qt::Horizontal,"endtime");
    model->setHeaderData(2,Qt::Horizontal,"starttemp");
    model->setHeaderData(3,Qt::Horizontal,"endtemp");
    model->setHeaderData(4,Qt::Horizontal,"speed");
    model->setHeaderData(5,Qt::Horizontal,"cost");
    t->setModel(model);
    t->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    t->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(4,QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(5,QHeaderView::Fixed);
 /*   ui->tableView->setColumnWidth(0,100);
    ui->tableView->setColumnWidth(1,100);
    ui->tableView->setColumnWidth(2,100);
    ui->tableView->setColumnWidth(3,100);
    ui->tableView->setColumnWidth(4,100);
    ui->tableView->setColumnWidth(5,100);*/
    data<<"2017/1/1/00:00"<<"2017/1/1/02:00"<<"26"<<"27"<<"high"<<"100"<<"end";
    QString starttime,endtime,starttemp,endtemp,speed,cost;
    int i=0,j=0,k=0;
    while(data.at(k)!="end"){
        starttime=data.at(k++);
        endtime=data.at(k++);
        starttemp=data.at(k++);
        endtemp=data.at(k++);
        speed=data.at(k++);
        cost=data.at(k++);
        model->setItem(i,j++,new QStandardItem(starttime));
        model->setItem(i,j++,new QStandardItem(endtime));
        model->setItem(i,j++,new QStandardItem(starttemp));
        model->setItem(i,j++,new QStandardItem(endtemp));
        model->setItem(i,j++,new QStandardItem(speed));
        model->setItem(i,j++,new QStandardItem(cost));
        j=0,i++;
    }
}
