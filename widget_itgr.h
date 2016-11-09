#ifndef WIDGET_ITGR_H
#define WIDGET_ITGR_H

#include <QWidget>
#include <QProgressBar>
#include <QAbstractTableModel>

#include "amper_dev.h"
#include "meter.h"

namespace Ui {
class widget_itgr;
}

class model_itgr : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit model_itgr(QObject *parent = 0);
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual void set_data(METER_INTEGRATORS *itgr);
protected:
    METER_INTEGRATORS itgr[TARIFF_MAX][PHASE_MAX];
    QVariant itgr_show(const uint64_t val) const;
};

class QTimer;
class widget_itgr : public QWidget
{
    Q_OBJECT

public:
    explicit widget_itgr(amper_dev *ad, QProgressBar *pb, QWidget *parent = 0);
    ~widget_itgr();
private:
    Ui::widget_itgr *ui;
protected:
    amper_dev * dev;
    QProgressBar *bar;

    QTimer * timer_start;
    METER_INTEGRATORS itgr[TARIFF_MAX][PHASE_MAX];
    void * dti_prop;
    model_itgr *model;
signals:
    void signal_read_data(int shift);
    void signal_read_end(bool ok);
public slots:
    void slot_start_after_ms(int start_delay_ms);
protected slots:
    void slot_timer_start();
    void slot_read_data(int shift);
};

#endif // WIDGET_ITGR_H
