#ifndef PAR_EDIT_H
#define PAR_EDIT_H

#include <QDialog>
#include <QTimer>
#include "amper_dev.h"
#include <QProgressBar>


namespace Ui {
class Par_edit;
}

class widget_METER_ARCH_DATA_SETTTINGS;
class par_edit : public QDialog
{
    Q_OBJECT

public:
    explicit par_edit(amper_dev *ad, QProgressBar *pb, QWidget *parent = 0);
    ~par_edit();

//    void setAmperDev(amper_dev *ad) {dev = ad;}
//    void setProgressBar(QProgressBar *pb) {bar = pb;}
    void readAllData();

private slots:
    void timeSetClicked(bool e);
    void timeReadTimer();
    void slot_SENSORS_upd();
    void on_checkBox_3_clicked(bool checked);
    void on_pushButton_3_clicked();
    void stopall(int);

    void on_pushButton_6_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_calibr_A_clicked();

    void on_pushButton_calibr_B_clicked();

    void on_pushButton_calibr_C_clicked();

    void on_pushButton_calibr_N_clicked();

    void on_pushButton_upd_sensors_clicked(bool checked);

private:
    Ui::Par_edit *ui;
    amper_dev * dev;
    QProgressBar *bar;
    QTimer *tm3;
    widget_METER_ARCH_DATA_SETTTINGS * wdg_profile_power_settings;
};

#endif // PAR_EDIT_H
