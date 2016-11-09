#include "widget_itgr.h"
#include "ui_widget_itgr.h"

#include <QTimer>

#include "logger.h"

widget_itgr::widget_itgr(amper_dev *ad, QProgressBar *pb, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::widget_itgr)
    , dev(ad)
    , bar(pb)
    , dti_prop(0)
{
    ui->setupUi(this);
    timer_start = new QTimer(this);
    timer_start->setSingleShot(true);
    connect(timer_start, SIGNAL(timeout()), this, SLOT(slot_timer_start()));

    model = new model_itgr(this);
    ui->tableView->setModel(model);

    connect(this, SIGNAL(signal_read_data(int)), this, SLOT(slot_read_data(int)), Qt::QueuedConnection);
}

widget_itgr::~widget_itgr()
{
    delete ui;
}

void widget_itgr::slot_start_after_ms(int start_delay_ms)
{
    timer_start->start(start_delay_ms);
}

void widget_itgr::slot_timer_start()
{
    if (!dti_prop) {
        dti_prop = new DTI_Property;
    }

    if (dev->getPropertyByName("pM_INTEGS", (DTI_Property *)dti_prop)) {
        emit signal_read_data(0);
        return;
    }
    log_1 << "no DTI_Property.name pM_INTEGS";
    emit signal_read_end(false);
}

void widget_itgr::slot_read_data(int shift)
{
    if (!dti_prop) return;
    DTI_Property * prop = (DTI_Property *)dti_prop;
    int communication_len = 100;
    uchar buff[communication_len];

    if (dev->readData(dev->propertySeg(prop), prop->address + shift, communication_len, buff)) {
        if (shift + communication_len < (int)sizeof(itgr)) {
            bar->setValue(shift * 100 / sizeof(itgr));
            memmove(((uchar*)&itgr) + shift, buff, communication_len);
            emit signal_read_data(shift + communication_len);
        } else {
            bar->setValue(100);
            memmove(((uchar*)&itgr) + shift, buff, sizeof(itgr) - shift);
            emit signal_read_end(true);
            model->set_data((METER_INTEGRATORS *)&itgr);
            ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
        }
    } else {
        emit signal_read_data(shift);
    }
}


model_itgr::model_itgr(QObject *parent)
    : QAbstractTableModel(parent)
{
    memset(&itgr, 0, sizeof(itgr));
}

int model_itgr::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 9;
}

int model_itgr::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return TARIFF_MAX * PHASE_MAX;
}

QVariant model_itgr::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole: {
        int tariff = index.column() % TARIFF_MAX;
        int phase  = index.column() / TARIFF_MAX;
        if (tariff >= TARIFF_MAX || phase >= PHASE_MAX) break;
        const METER_INTEGRATORS * itg = &itgr[tariff][phase];
        switch (index.row()) {
        case 0: return itgr_show(itg->P_p);
        case 1: return itgr_show(itg->P_n);
        case 2: return itgr_show(itg->Q_p);
        case 3: return itgr_show(itg->Q_n);
        case 4: return itgr_show(itg->S);
        case 5: return itgr_show(itg->Q1);
        case 6: return itgr_show(itg->Q2);
        case 7: return itgr_show(itg->Q3);
        case 8: return itgr_show(itg->Q4);
        default: break;
        }
        break;
    }
    case Qt::TextAlignmentRole: return Qt::AlignRight + Qt::AlignVCenter;
    default: break;
    }

    return QVariant();
}

QVariant model_itgr::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QAbstractTableModel::headerData(section, orientation, role);
    if (orientation != Qt::Horizontal) {
        switch (section) {
        case 0: return tr("Активная +");
        case 1: return tr("Активная -");
        case 2: return tr("Реактивная +");
        case 3: return tr("Реактивная -");
        case 4: return tr("Полная");
        case 5: return tr("QI");
        case 6: return tr("QII");
        case 7: return tr("QIII");
        case 8: return tr("QIV");
        default: break;
        }
    } else {
        switch (section) {
        case 0: return tr("Σ тарифов\nΣ фаз");
        case 1: return tr("Тариф 1\nΣ фаз");
        case 2: return tr("Тариф 2\nΣ фаз");
        case 3: return tr("Тариф 3\nΣ фаз");
        case 4: return tr("Тариф 4\nΣ фаз");
        case 5: return tr("Σ тарифов\nФаза 1");
        case 6: return tr("Тариф 1\nФаза 1");
        case 7: return tr("Тариф 2\nФаза 1");
        case 8: return tr("Тариф 3\nФаза 1");
        case 9: return tr("Тариф 4\nФаза 1");
        case 10: return tr("Σ тарифов\nФаза 2");
        case 11: return tr("Тариф 1\nФаза 2");
        case 12: return tr("Тариф 2\nФаза 2");
        case 13: return tr("Тариф 3\nФаза 2");
        case 14: return tr("Тариф 4\nФаза 2");
        case 15: return tr("Σ тарифов\nФаза 3");
        case 16: return tr("Тариф 1\nФаза 3");
        case 17: return tr("Тариф 2\nФаза 3");
        case 18: return tr("Тариф 3\nФаза 3");
        case 19: return tr("Тариф 4\nФаза 3");
        default: break;
        }
    }
    return QVariant();
}

void model_itgr::set_data(METER_INTEGRATORS *itgr)
{
    layoutAboutToBeChanged();
    if (!itgr) {
        memset(this->itgr, 0, sizeof(this->itgr));
    } else {
        memmove(this->itgr, itgr, sizeof(this->itgr));
    }
    layoutChanged();
}

QVariant model_itgr::itgr_show(const uint64_t val) const
{
    return QString::number(((double)val) / 2000.0 / 1000.0, 'f', 3);
}
