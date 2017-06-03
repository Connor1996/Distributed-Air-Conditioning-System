#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <QDialog>
#include "ui_configure.h"

namespace Ui {
class Configure;
}

class Configure : public QDialog
{
    Q_OBJECT

public:
    explicit Configure(QWidget *parent = 0)
     : QDialog(parent), ui(new Ui::Configure)
    {
        ui->setupUi(this);
        connect(ui->cancelButtion, &QPushButton::clicked, [this](){
            this->close();
        });

        connect(ui->saveButton, &QPushButton::clicked, [this](){
            emit change(
                        ui->serveEdit->value(),
                        ui->frequenceEdit->value()
                        );
            this->close();
        });
    }

    ~Configure() { delete ui; }

signals:
    void change(int, int);

private:
    Ui::Configure *ui;
};

#endif // CONFIGURE_H
