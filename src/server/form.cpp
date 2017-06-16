#include "form.h"
#include "ui_form.h"
#include "request.h"


Form::Form(Server *server, QWidget *parent) :
    _server(server),
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    InitConnect();

    ui->tabWidget->setCurrentIndex(0);

    ui->toolBox->setCurrentIndex(2);
    ui->toolBox_2->setCurrentIndex(2);
    ui->toolBox_3->setCurrentIndex(2);

    int bar_room[8];
    for(int i=0; i<8; i++) {
        //bar_room[i] = 1;

        bar_room[i] = _server->GetRoomCount(411+i);
    }
    barinit(ui->page_1, bar_room);
    barinit(ui->page_4, bar_room);
    barinit(ui->page_7, bar_room);

    QStringList combo_room;
    combo_room << "room" << "411" << "412" << "413" << "414" << "415" <<
                  "416" << "417" << "418";
    ui->comboBox->addItems(combo_room);
    ui->comboBox_2->addItems(combo_room);
    ui->comboBox_3->addItems(combo_room);

    double room[8];
    double total = 0;
    for(int i=0; i<8; i++){
        room[i] = getcost(411+i);
        total += room[i];
    }
    ui->label->setText("total: " + QString::number(total, 'g', 6));
    ui->label_2->setText("total: " + QString::number(total, 'g', 6));
    ui->label_3->setText("total: " + QString::number(total, 'g', 6));

    //page1 饼图
    piescene = new QGraphicsScene();
    pieview = new QGraphicsView(ui->page_3);
    pieview->setScene(piescene);
    pieview->setRenderHint(QPainter::Antialiasing);
    piescene->setBackgroundBrush(QBrush(QColor(240, 240, 240)));
    pieview->setSceneRect(10, 10, 400, 300);
    pieview->setSizeIncrement(400,300);
    pieview->resizeAnchor();

    pieSeries = new QPieSeries();
    pieSeries->append("411", room[0]/total);
    pieSeries->append("412", room[1]/total);
    pieSeries->append("413", room[2]/total);
    pieSeries->append("414", room[3]/total);
    pieSeries->append("415", room[4]/total);
    pieSeries->append("416", room[5]/total);
    pieSeries->append("417", room[6]/total);
    pieSeries->append("418", room[7]/total);

    pieChart = new QChart();
    pieChart->addSeries(pieSeries);  // 将 series 添加至图表中
    pieChart->legend()->setAlignment(Qt::AlignRight);  // 设置图例靠右显示
    pieChart->setTitle("Day Pie Chart");
    pieChart->setGeometry(10, 10, 400, 300);
    piescene->addItem(pieChart);
    //room_cost table
    model3 = new QStandardItemModel();
    model3->setColumnCount(2);
    model3->setHeaderData(0,Qt::Horizontal,"roomid");
    model3->setHeaderData(1,Qt::Horizontal,"cost");

    ui->costtable->setModel(model3);
    ui->costtable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    ui->costtable->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);
    ui->costtable->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Fixed);

    for(int i=0; i<8; i++){
        model3->setItem(i, 0, new QStandardItem(QString::number(411+i)));
        model3->setItem(i, 1, new QStandardItem(QString::number(room[i], 'g', 6)));
    }

    //page2 折线图
    linescene = new QGraphicsScene();
    lineview = new QGraphicsView(ui->page_6);
    lineview->setScene(linescene);
    lineview->setRenderHint(QPainter::Antialiasing);
    linescene->setBackgroundBrush(QBrush(QColor(240, 240, 240)));
    lineview->setSceneRect(10, 10,560, 300);
    lineview->setSizeIncrement(560,300);
    lineview->resizeAnchor();

    QCategoryAxis *axisX = new QCategoryAxis();
    QCategoryAxis *axisY = new QCategoryAxis();
    //axisY->setRange(0,6);
    axisX->setRange(0,7);


    // 构建 series，作为图表的数据源
    lineseries = new QLineSeries();
    /*for(int i=0; i<7; i++){
        lineseries->append(i+1, i%3);
    }*/
    lineseries->append(0, 0);
    lineseries->append(1, total);
    lineChart = new QChart();
    lineChart->addSeries(lineseries);  // 将 series 添加至图表中
    lineChart->setAxisX(axisX, lineseries);
    lineChart->setAxisY(axisY, lineseries);

    lineChart->legend()->hide();
    lineChart->createDefaultAxes();
    lineChart->setTitle("Week Line Chart");
    lineChart->setGeometry(10, 10, 560, 300);
    linescene->addItem(lineChart);

    //page3 折线图
    monthlinescene = new QGraphicsScene();
    monthlineview = new QGraphicsView(ui->page_9);
    monthlineview->setScene(monthlinescene);
    monthlineview->setRenderHint(QPainter::Antialiasing);
    monthlinescene->setBackgroundBrush(QBrush(QColor(240, 240, 240)));
    monthlineview->setSceneRect(10, 10,560, 300);
    monthlineview->setSizeIncrement(560,300);
    monthlineview->resizeAnchor();

    QCategoryAxis *monthaxisX = new QCategoryAxis();
    QCategoryAxis *monthaxisY = new QCategoryAxis();
    //monthaxisY->setRange(0,6);
    monthaxisX->setRange(0,30);
    monthaxisX->setLabelFormat("%d");
    // 构建 series，作为图表的数据源
    QLineSeries *monthlineseries = new QLineSeries();
    /*for(int i=0; i<31; i++){
        monthlineseries->append(i, i%6);
    }*/
    monthlineseries->append(0, 0);
    monthlineseries->append(1, total);

    monthlineChart = new QChart();
    monthlineChart->addSeries(monthlineseries);  // 将 series 添加至图表中
    monthlineChart->setAxisX(monthaxisX, monthlineseries);
    monthlineChart->setAxisY(monthaxisY, monthlineseries);

    monthlineChart->legend()->hide();
    monthlineChart->createDefaultAxes();
    monthlineChart->setTitle("Month Line Chart");
    monthlineChart->setGeometry(10, 10, 560, 300);
    monthlinescene->addItem(monthlineChart);
}

Form::~Form()
{
    delete piescene;
    delete pieview;
    delete pieSeries;
    delete pieChart;
    delete model3;
    delete linescene;
    delete lineview;
    delete lineseries;
    delete lineChart;
    delete monthlinescene;
    delete monthlineview;
    delete monthlineseries;
    delete monthlineChart;
    delete ui;
}

void Form::InitConnect() {
    connect(ui->comboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged)
            , [this](QString index){
                    if(index != "room") {
                        //std::vector<Request> data;
                        //data = _server->GetRoomRequests(index.toInt());
                        tableinit(ui->tableView, _server->GetRoomRequests(index.toInt()));
                    }
    });
    connect(ui->comboBox_2, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged)
            , [this](QString index){
                    if(index != "room") {
                        //std::vector<Request> data;
                        //data = _server->GetRoomRequests(index.toInt());
                        tableinit(ui->tableView_2, _server->GetRoomRequests(index.toInt()));
                    }
    });
    connect(ui->comboBox_3, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged)
            , [this](QString index){
                    if(index != "room") {
                        //std::vector<Request> data;
                        //data = _server->GetRoomRequests(index.toInt());
                        tableinit(ui->tableView_3, _server->GetRoomRequests(index.toInt()));
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
    barchart->setTitle("ON/OFF bar chart");
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

void Form::tableinit(QTableView *t, std::vector<Request> data){
    //tableview
    QStandardItemModel *model = new QStandardItemModel();

    model->setColumnCount(7);
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
    model->setHeaderData(4,Qt::Horizontal,"startspeed");
    model->setHeaderData(5,Qt::Horizontal,"endspeed");
    model->setHeaderData(6,Qt::Horizontal,"cost");
    t->setModel(model);
    t->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    t->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(4,QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(5,QHeaderView::Fixed);
    t->horizontalHeader()->setSectionResizeMode(6,QHeaderView::Fixed);

 /*   ui->tableView->setColumnWidth(0,100);
    ui->tableView->setColumnWidth(1,100);
    ui->tableView->setColumnWidth(2,100);
    ui->tableView->setColumnWidth(3,100);
    ui->tableView->setColumnWidth(4,100);
    ui->tableView->setColumnWidth(5,100);*/

    QString starttime,endtime,starttemp,endtemp,startspeed,endspeed,cost;
    for(int i=0; i<data.size(); i++){
        starttime = QString::number(data[i].beginTime, 10);
        endtime = QString::number(data[i].stopTime, 10);
        starttemp = QString::number(data[i].beginTemp, 10);
        endtemp = QString::number(data[i].stopTemp, 10);
        startspeed = QString::number(data[i].beginSpeed, 10);
        endspeed = QString::number(data[i].stopSpeed, 10);
        cost = QString::number(data[i].money, 'g', 6);
        model->setItem(i,0,new QStandardItem(starttime));
        model->setItem(i,1,new QStandardItem(endtime));
        model->setItem(i,2,new QStandardItem(starttemp));
        model->setItem(i,3,new QStandardItem(endtemp));
        model->setItem(i,4,new QStandardItem(startspeed));
        model->setItem(i,5,new QStandardItem(endspeed));
        model->setItem(i,6,new QStandardItem(cost));
    }
}

double Form::getcost(int r){
    std::vector<Request> data = _server->GetRoomRequests(r);
    double total = 0.0;
    if(data.size()!=0){
        for(int i=0; i<data.size(); i++){
            total += data[i].money;
        }
    }
    return total;
}
