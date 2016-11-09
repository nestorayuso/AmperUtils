#ifndef WIDGET_METER_CURR_H
#define WIDGET_METER_CURR_H

#include <QWidget>
#include <QProgressBar>
#include <QAbstractTableModel>

#include "amper_dev.h"
#include "meter.h"

namespace Ui {
class widget_meter_curr;
}

class model_meter_curr : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit model_meter_curr(QObject *parent = 0);
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual void set_data(METER_NEW_DATA *curr);
protected:
    METER_NEW_DATA curr;
    QVariant itgr_show(const float val) const;
};


class QTimer;
class widget_meter_curr : public QWidget
{
    Q_OBJECT

public:
    explicit widget_meter_curr(amper_dev *ad, QProgressBar *pb, QWidget *parent = 0);
    ~widget_meter_curr();

private:
    Ui::widget_meter_curr *ui;
protected:
    amper_dev * dev;
    QProgressBar *bar;

    QTimer * timer_start;
    METER_NEW_DATA curr;
    void * dti_prop;
    model_meter_curr *model;
signals:
    void signal_read_data(int shift);
    void signal_read_end(bool ok);
public slots:
    void slot_start_after_ms(int start_delay_ms);
protected slots:
    void slot_timer_start();
    void slot_read_data(int shift);
};

#endif // WIDGET_METER_CURR_H
