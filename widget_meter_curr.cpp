#include "widget_meter_curr.h"
#include "ui_widget_meter_curr.h"

#include <QTimer>

#include "logger.h"

widget_meter_curr::widget_meter_curr(amper_dev *ad, QProgressBar *pb, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::widget_meter_curr)
    , dev(ad)
    , bar(pb)
    , dti_prop(0)
{
    ui->setupUi(this);
    timer_start = new QTimer(this);
    timer_start->setSingleShot(true);
    connect(timer_start, SIGNAL(timeout()), this, SLOT(slot_timer_start()));

    model = new model_meter_curr(this);
    ui->tableView->setModel(model);

    connect(this, SIGNAL(signal_read_data(int)), this, SLOT(slot_read_data(int)), Qt::QueuedConnection);
}

widget_meter_curr::~widget_meter_curr()
{
    delete ui;
}

void widget_meter_curr::slot_start_after_ms(int start_delay_ms)
{
    timer_start->start(start_delay_ms);
}

void widget_meter_curr::slot_timer_start()
{
    if (!dti_prop) {
        dti_prop = new DTI_Property;
    }

    if (dev->getPropertyByName("pM_CURR", (DTI_Property *)dti_prop)) {
        emit signal_read_data(0);
        return;
    }
    log_1 << "no DTI_Property.name pM_CURR";
    emit signal_read_end(false);
}

void widget_meter_curr::slot_read_data(int shift)
{
    if (!dti_prop) return;
    DTI_Property * prop = (DTI_Property *)dti_prop;
    int communication_len = 100;
    uchar buff[communication_len];

    if (dev->readData(dev->propertySeg(prop), prop->address + shift, communication_len, buff)) {
        if (shift + communication_len < (int)sizeof(curr)) {
            bar->setValue(shift * 100 / sizeof(curr));
            memmove(((uchar*)&curr) + shift, buff, communication_len);
            emit signal_read_data(shift + communication_len);
        } else {
            bar->setValue(100);
            memmove(((uchar*)&curr) + shift, buff, sizeof(curr) - shift);
            emit signal_read_end(true);
            model->set_data((METER_NEW_DATA *)&curr);
            ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
        }
    } else {
        emit signal_read_data(shift);
    }
}

model_meter_curr::model_meter_curr(QObject *parent)
    : QAbstractTableModel(parent)
{
    memset(&curr, 0, sizeof(curr));
}

int model_meter_curr::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 8;
}

int model_meter_curr::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return PHASE_MAX;
}

QVariant model_meter_curr::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole: {
        int phase = index.column();
        if (phase >= PHASE_MAX) break;
        switch (index.row()) {
        case 0: return itgr_show(curr.to_display[phase].Pi / 1000.0);
        case 1: return itgr_show(curr.to_display[phase].Qi / 1000.0);
        case 2: return itgr_show(curr.to_display[phase].Si / 1000.0);
        case 3: return itgr_show(curr.phase[phase].U);
        case 4: return itgr_show(curr.phase[phase].Ulin);
        case 5: return itgr_show(curr.phase[phase].I);
        case 6: return itgr_show(curr.phase[phase].F);
        case 7: return itgr_show(curr.phase[phase].cosFi);
        default: break;
        }
        break;
    }
    case Qt::TextAlignmentRole: return Qt::AlignRight + Qt::AlignVCenter;
    default: break;
    }

    return QVariant();
}

QVariant model_meter_curr::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QAbstractTableModel::headerData(section, orientation, role);
    if (orientation != Qt::Horizontal) {
        switch (section) {
        case 0: return tr("Мощность Активная");
        case 1: return tr("Мощность Реактивная");
        case 2: return tr("Мощность Полная");
        case 3: return tr("Напряжение RMS");
        case 4: return tr("Напряжение линейное RMS");
        case 5: return tr("Ток RMS");
        case 6: return tr("Частота сети");
        case 7: return tr("cosFi");
        default: break;
        }
    } else {
        switch (section) {
        case 0: return tr("Σ фаз");
        case 1: return tr("Фаза 1");
        case 2: return tr("Фаза 2");
        case 3: return tr("Фаза 3");
        default: break;
        }
    }
    return QVariant();
}

void model_meter_curr::set_data(METER_NEW_DATA *curr)
{
    layoutAboutToBeChanged();
    if (!curr) {
        memset(&this->curr, 0, sizeof(this->curr));
    } else {
        memmove(&this->curr, curr, sizeof(this->curr));
    }
    layoutChanged();
}

QVariant model_meter_curr::itgr_show(const float val) const
{
    return QString::number(val, 'f', 3);
}
