#ifndef TARIF_EDIT_H
#define TARIF_EDIT_H

#include <QDialog>
#include "amper_dev.h"
#include <QProgressBar>
#include <QCheckBox>
#include "widget_tariff_edit.h"

namespace Ui {
class tarif_edit;
}

class tarif_edit : public QDialog
{
    Q_OBJECT

public:
    explicit tarif_edit(amper_dev *ad, QProgressBar *pb, QWidget *parent = 0);
    ~tarif_edit();
    void setAmperDev(amper_dev *ad) {dev = ad;}
    void setProgressBar(QProgressBar *pb) {bar = pb;}
    void setZoneButton(quint32 time, RateZoneType_e name);
    void setZoneButton(uint8_t zone, uint8_t tariff);
    quint64 getZoneBitPack(RateZoneType_e name);
    void readAllData();
public slots:
    void butToggled(bool e);
    void readClicked();
    void writeClicked();
    void checkBoxClicked();

private slots:
    void on_pushButton_3_clicked();

private:
    Ui::tarif_edit *ui;
    QList<QPushButton *> allPButtons_1, allPButtons_2;
    QList<QCheckBox *> all_checkboxes;
    amper_dev * dev;
    QProgressBar *bar;
    void *ptariff;
    widget_tariff_edit *wte;
};

#endif // TARIF_EDIT_H
