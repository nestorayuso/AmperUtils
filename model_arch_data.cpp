#include "model_arch_data.h"

#include <QVariantMap>
#include <QDateTime>
#include <QBrush>
#include <QTimeZone>
#include <QTimer>
#include <QEventLoop>

#include "meter.h"
#include "logger.h"

Q_DECLARE_METATYPE(METER_ARCH_DATA);
Q_DECLARE_METATYPE(METER_ARCH_DATA_SETTINGS);

model_arch_data::model_arch_data(amper_dev *dev, QObject *parent)
    : QAbstractTableModel(parent)
    , dev(dev)
    , can_use_dev(0)
    , is_use_dev(0)
    , communication_len(100)
    , record_len(sizeof(METER_ARCH_DATA))
{
    qRegisterMetaType<METER_ARCH_DATA>("METER_ARCH_DATA");
    connect(this, SIGNAL(signal_cont_readFlash(int,int)), this, SLOT(slot_readFlash(int,int)), Qt::QueuedConnection);
}

int model_arch_data::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return datalist.count();
}

int model_arch_data::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return ARCH_DATA_PARAM_COUT_MAX + 1;
}

QVariant model_arch_data::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole: {
        if (index.row() < 0 || index.row() >= datalist.count()) break;
        METER_ARCH_DATA mad = datalist.value(index.row()).value<METER_ARCH_DATA>();
        switch (index.column()) {
        //case 0: return mad.timestamp;//QDateTime::fromTime_t(mad.timestamp, QTimeZone(0)).toString("yyyy.MM.dd hh.mm");
        case 0: return QDateTime::fromTime_t(mad.timestamp, QTimeZone(0)).toString("yyyy.MM.dd hh.mm");
        default: {
            METER_ARCH_DATA_SETTINGS mads = header_settings.value<METER_ARCH_DATA_SETTINGS>();
            METER_DATA_TYPE mdt = (METER_DATA_TYPE)mads.param_id[index.column() - 1].MDT;
            float val = 0;
            switch (mdt) {
            case MDT_P_p    :
            case MDT_P_p_d  :
            case MDT_P_n    :
            case MDT_P_n_d  :
            case MDT_Q_p    :
            case MDT_Q_p_d  :
            case MDT_Q_n    :
            case MDT_Q_n_d  :
            case MDT_S      :
            case MDT_S_d    :
            case MDT_Q1     :
            case MDT_Q2     :
            case MDT_Q3     :
            case MDT_Q4     : {
                val = mad.value[index.column()-1].f32 / 1000.0;
                break;
//                return QString("%1").arg(((double)mad.value[index.column()-1].u32) / 2000.0 / 1000.0, 0, 'f', 3, QChar('0')) ;
            }
            case MDT_U      :
            case MDT_Ulin   :
            case MDT_I      :
            case MDT_F      :
            case MDT_cosFi  : {
                val = mad.value[index.column()-1].f32;
                break;
            }
            default: break;
            }
            return QString("%1").arg(val, 0, 'f', 3);
        }
        }

        //QString("val%1").arg(i)
        break;
    }
    case Qt::TextAlignmentRole: return Qt::AlignRight + Qt::AlignVCenter; //QAbstractTableModel::headerData(section, orientation, role);
    default: break;
    }
    return QVariant();
}

QVariant model_arch_data::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) return QVariant();
    switch (role) {
    case Qt::DisplayRole: {
        METER_ARCH_DATA_SETTINGS mads = header_settings.value<METER_ARCH_DATA_SETTINGS>();
        switch (section) {
        case 0: return tr("Время записи");
        default: {
            METER_DATA_TYPE mdt = (METER_DATA_TYPE)mads.param_id[section - 1].MDT;
            QString strh;
            switch (mdt) {
            case MDT_P_p    : strh+= tr("A+"); break;
            case MDT_P_p_d  : strh+= tr("A+d"); break;
            case MDT_P_n    : strh+= tr("A-"); break;
            case MDT_P_n_d  : strh+= tr("A-d"); break;
            case MDT_Q_p    : strh+= tr("R+"); break;
            case MDT_Q_p_d  : strh+= tr("R+d"); break;
            case MDT_Q_n    : strh+= tr("R-"); break;
            case MDT_Q_n_d  : strh+= tr("R-d"); break;
            case MDT_S      : strh+= tr("ПОЛНАЯ"); break;
            case MDT_S_d    : strh+= tr("ПОЛНАЯd"); break;
            case MDT_Q1     : strh+= tr("QI"); break;
            case MDT_Q2     : strh+= tr("QII"); break;
            case MDT_Q3     : strh+= tr("QIII"); break;
            case MDT_Q4     : strh+= tr("QIV"); break;
            case MDT_U      : strh+= tr("Uф"); break;
            case MDT_Ulin   : strh+= tr("Uл"); break;
            case MDT_I      : strh+= tr("I"); break;
            case MDT_F      : strh+= tr("F"); break;
            case MDT_cosFi  : strh+= tr("cosFi"); break;
            default: break;
            }
            strh += "\n";
            switch (mads.param_id[section - 1].phase) {
            case 0: strh+= "Σ фаз"; break;
            case 1: strh+= "фаз 1"; break;
            case 2: strh+= "фаз 2"; break;
            case 3: strh+= "фаз 3"; break;
            default: break;
            }
            strh += "\n";
            switch (mads.param_id[section - 1].tariff) {
            case 0: strh+= "Σ тарифов"; break;
            case 1: strh+= "тариф 1"; break;
            case 2: strh+= "тариф 2"; break;
            case 3: strh+= "тариф 3"; break;
            case 4: strh+= "тариф 4"; break;
            default: break;
            }
            return strh;
        }
        }
        break;
    }
    case Qt::TextAlignmentRole: return QAbstractTableModel::headerData(section, orientation, role);
//    case Qt::BackgroundRole: return QBrush(QColor(Qt::darkGreen));//QAbstractTableModel::headerData(section, orientation, role);
    case Qt::BackgroundRole: return QAbstractTableModel::headerData(section, orientation, role);
    case Qt::ForegroundRole: return QAbstractTableModel::headerData(section, orientation, role);
    case Qt::FontRole: return QAbstractTableModel::headerData(section, orientation, role);
    case Qt::DecorationRole: return QAbstractTableModel::headerData(section, orientation, role);
    case Qt::SizeHintRole: return QAbstractTableModel::headerData(section, orientation, role);
    }
    log_1 << role;
    return QAbstractTableModel::headerData(section, orientation, role);
}

uint8_t model_arch_data::ARCH_crc8(const void *vptr, int len)
{
    const uint8_t *data = (uint8_t *)vptr;
    unsigned crc = 0;
    int i, j;
    for (j = len; j; j--, data++) {
        crc ^= (*data << 8);
        for(i = 8; i; i--) {
            if (crc & 0x8000)
                crc ^= (0x1070 << 3);
            crc <<= 1;
        }
    }
    return (uint8_t)(crc >> 8);
}

void model_arch_data::slot_readFlash(int addr_start, int len)
{
    is_use_dev = false;
    if (!dev) return;
    if (!can_use_dev) return;
//    if (can_use_dev && len) {
//        log_1 << "skip";
//        return;
//    }
//    if (!len && !addr_start) {
//        can_use_dev = 1;
//        log_1 << "stop";
//        return;
//    }
//    log_1 << addr_start << len;
//    can_use_dev = 0;
    uint ln = 0;
    QByteArray arr;
    bool is_ok = true;
    if (!len) {
        //emit signal_readFlash(addr_start, QByteArray());
        return;
    }
    is_use_dev = true;
    ln = (uint)len > communication_len ? communication_len : (uint)len;
    if (arr.size() != (int)ln) arr.resize(ln);
    is_ok = dev->readData(EE_RW, addr_start, arr.size(), arr.data());
    if (is_ok && can_use_dev) {
        add(addr_start, arr);
        emit signal_persent_complated(progress_load());
        //emit signal_readFlash(addr_start, arr);
        addr_start += ln;
        len -= ln;
        emit signal_cont_readFlash(addr_start, len);
    }
}

void model_arch_data::add(QByteArray data)
{
    if (data.size() != record_len) return;
    METER_ARCH_DATA * mad = (METER_ARCH_DATA*)data.data();
    if (ARCH_crc8(mad, record_len - 1) != mad->crc) {
        //log_1 << "error crc" << mad->crc << ARCH_crc8(mad, record_len - 1);
        return;
    }
    if (!mad->timestamp) {
        return;
    }
    layoutAboutToBeChanged();
    datalist.append(QVariant::fromValue(*mad));
    layoutChanged();
    log_1 << "datalist.count" << datalist.count();
}

void model_arch_data::connect_dev(bool can_use)
{
//    disconnect(this, SIGNAL(signal_cont_readFlash(int,int)), this, SLOT(slot_readFlash(int,int)));
    can_use_dev = can_use;
//    connect(this, SIGNAL(signal_cont_readFlash(int,int)), this, SLOT(slot_readFlash(int,int)), Qt::QueuedConnection);
//    while(is_use_dev) {
//        QEventLoop loop;
//        QTimer::singleShot(1000, &loop, SLOT(quit()));
//        loop.exec();
//        log_1 << "loop";
//    }
}

void model_arch_data::add(int addr, QByteArray data)
{
    if (!data.size()) {
        log_1 << "no data";
        return;
    }
    read_buff.append(data);
    while (read_buff.size() >= record_len) {
        add(read_buff.left(record_len));
        read_buff.remove(0, record_len);
    }
    flash_curr_addr = addr + data.size();
    //log_1 << "flash_curr_addr" << flash_curr_addr;
}

float model_arch_data::progress_load()
{
    return ((float)(flash_curr_addr - flash_start_addr)) / flash_len;
}

void model_arch_data::set_header(QByteArray data)
{
    if (data.size() != sizeof(METER_ARCH_DATA_SETTINGS)) return;
    METER_ARCH_DATA_SETTINGS * mads = (METER_ARCH_DATA_SETTINGS*)data.data();
    layoutAboutToBeChanged();
    header_settings = QVariant::fromValue(*mads);
    layoutChanged();
}

void model_arch_data::clear()
{
    layoutAboutToBeChanged();
    datalist.clear();
    can_use_dev = false;

    read_buff.clear();
    flash_curr_addr = flash_start_addr;
    log_1 << "flash_curr_addr" << flash_curr_addr;
    layoutChanged();
}
