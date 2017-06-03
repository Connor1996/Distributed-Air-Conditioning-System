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
    }

    ~Configure() { delete ui; }

private:
    Ui::Configure *ui;
};

#endif // CONFIGURE_H
