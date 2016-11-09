#ifndef AUTOCAL_EDIT_H
#define AUTOCAL_EDIT_H

#include <QDialog>
#include "amper_dev.h"
#include <QProgressBar>

#define ETALON_ADD  100
#define U_CAL_TIME  (20)
#define I_CAL_TIME  (20)
#define E_CAL_WH    75


#define AMPER_SUPPLY_CURRENT    0.0136
#define AMPER_SUPPLY_APOWER    1.2
#define AMPER_SUPPLY_RPOWER    3.2


namespace Ui {
class autocal_edit;
}

class autocal_edit : public QDialog
{
    Q_OBJECT

public:
    explicit autocal_edit(amper_dev *ad, QProgressBar *pb, int phase, QWidget *parent = 0);
    ~autocal_edit();
    void setAmperDev(amper_dev *ad) {dev = ad;}
    void setProgressBar(QProgressBar *pb) {bar = pb;}

    void sendText2Progress(QString s);

private slots:
    void on_pushButton_2_clicked();
    bool stage_1();
    bool stage_2();
    bool stage_3();
    bool stage_4();
    bool stage_5();
    bool stage_6();

    void on_pushButton_3_clicked();

private:
    bool delay(quint32 seconds);
    Ui::autocal_edit *ui;
    amper_dev * dev;
    QProgressBar *bar;
    amper_dev * devices[256];
    quint8 dev_num;
    bool stop;
    int phase;
};

#endif // AUTOCAL_EDIT_H
