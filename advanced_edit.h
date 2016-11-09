#ifndef ADVANCED_EDIT_H
#define ADVANCED_EDIT_H

#include <QDialog>
#include "amper_dev.h"
#include "autocal_edit.h"
#include <QProgressBar>
#include <QTimer>

namespace Ui {
class advanced_edit;
}

class advanced_edit : public QDialog
{
    Q_OBJECT

public:
    explicit advanced_edit(amper_dev *ad, QProgressBar *pb, int phase, float K, QWidget *parent = 0);
    ~advanced_edit();
//    void setAmperDev(amper_dev *ad) {dev = ad;}
//    void setProgressBar(QProgressBar *pb) {bar = pb;}

    uint32_t new_ADE_Cal_record(uint32_t old, double mul);
protected:
    bool ADE_Cal_to_ui();
    bool ui_to_ADE_Cal();
private slots:
    //void on_pushButton_2_clicked();

    //void on_pushButton_3_clicked();

    //void on_pushButton_4_clicked();

    void on_pushButton_read_curr_clicked();

    void currentReadTimer();
    void secundomer();

    void on_pushButton_7_clicked();

//    void on_pushButton_9_clicked();

//    void on_pushButton_9_toggled(bool checked);

    void on_pushButton_8_clicked();

    void on_toolButton_clicked();

    void on_pushButton_4_clicked();

    void on_toolButton_2_clicked();

    void on_pushButton_read_ADE_Cal_clicked();

    void on_pushButton_write_ADE_Cal_clicked();

    void on_checkBox_read_curr_loop_clicked(bool checked);

    void on_pushButton_start_stop_clicked(bool checked);

    void on_pushButton_releOn_clicked();

    void on_pushButton_releOff_clicked();

private:
    Ui::advanced_edit *ui;
    amper_dev * dev;
    QProgressBar *bar;
    QTimer *tm1, *tm2;
    quint64 starttime;
    quint64 endtime;
    int phase;
    float K;
};

#endif // ADVANCED_EDIT_H
