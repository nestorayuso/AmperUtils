#ifndef WIDGET_METER_ARCH_DATA_SETTTINGS_H
#define WIDGET_METER_ARCH_DATA_SETTTINGS_H

#include <QWidget>
#include "meter.h"
#include "amper_dev.h"

namespace Ui {
class widget_METER_ARCH_DATA_SETTTINGS;
}

class widget_METER_ARCH_DATA_SETTTINGS : public QWidget
{
    Q_OBJECT

public:
    explicit widget_METER_ARCH_DATA_SETTTINGS(amper_dev *ad, QWidget *parent = 0);
    ~widget_METER_ARCH_DATA_SETTTINGS();

    METER_ARCH_DATA_SETTINGS mads;
private:
    Ui::widget_METER_ARCH_DATA_SETTTINGS *ui;
protected:
    amper_dev * dev;
public slots:
    void slot_update_ui_from_data();
    void slot_update_data_from_ui();
private slots:
    void on_pushButton_Save_clicked();
};

#endif // WIDGET_METER_ARCH_DATA_SETTTINGS_H
