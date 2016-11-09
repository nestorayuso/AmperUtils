#include "model_arch_search.h"

#include <QVariantMap>
#include <QDateTime>
#include <QBrush>
#include <QTimeZone>
#include <QTimer>
#include <QEventLoop>

#include "meter.h"
#include "logger.h"

model_arch_search::model_arch_search(amper_dev *dev, QObject *parent)
    : QAbstractTableModel(parent)
    , dev(dev)
    , flash_record_len(72)
    , can_use_dev(0)
    , is_use_dev(0)
    , last_date_time(0)
    , f_h_date_time(0)
    , f_l_date_time(0)
{
    connect(this, SIGNAL(signal_cont_readFlash(int,int)), this, SLOT(slot_readFlash(int,int)), Qt::QueuedConnection);
}

int model_arch_search::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return datalist.count();
}

int model_arch_search::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant model_arch_search::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole: {
        if (index.row() < 0 || index.row() >= datalist.count()) break;
        QVariantMap vmap = datalist.at(index.row()).toMap();
        switch (index.column()) {
        case 0: {
            if ((uint32_t)vmap.value("timestamp", 0).toInt() == 0xFFFFFFFF) {
                return "0xFFFFFFFF";
            }
            return QDateTime::fromTime_t(vmap.value("timestamp", 0).toInt(), QTimeZone(0)).toString("yyyy.MM.dd hh.mm.ss");
        }
        case 1: return vmap.value("addr", 0);
        }
        break;
    }
    case Qt::TextAlignmentRole: return Qt::AlignRight + Qt::AlignVCenter; //QAbstractTableModel::headerData(section, orientation, role);
    default: break;
    }
    return QVariant();
}

QVariant model_arch_search::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) return QVariant();
    switch (role) {
    case Qt::DisplayRole: {
        switch (section) {
        case 0: return tr("Время записи");
        case 1: return tr("Смещение");
        }
        break;
    }
    default: break;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

uint8_t model_arch_search::ARCH_crc8(const void *vptr, int len)
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

void model_arch_search::slot_readFlash(int addr_start, int len)
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
    log_1 << addr_start << len;
//    can_use_dev = 0;
    uint32_t ln = 0;
    uint32_t dt = 0;
    QByteArray arr;
    bool is_ok = true;
    if (!len) {
        //emit signal_readFlash(addr_start, QByteArray());
        return;
    }
    is_use_dev = true;
    ln = len > flash_record_len ? flash_record_len : len;
    ln = 4;
    if (arr.size() != (int)ln) arr.resize(ln);
    log_1 << addr_start << arr.size();
    if (!f_h_date_time) {
        log_1 << f_h;
        is_ok = dev->readData(EE_RW, f_h * flash_record_len + flash_start_addr, arr.size(), arr.data());
        f_h_date_time = *((uint32_t *)arr.data());
        add(f_h, arr);
    }
    if (!f_l_date_time) {
        log_1 << f_l;
        is_ok = dev->readData(EE_RW, f_l * flash_record_len + flash_start_addr, arr.size(), arr.data());
        f_l_date_time = *((uint32_t *)arr.data());
        add(f_l, arr);
    }
    is_ok = dev->readData(EE_RW, addr_start * flash_record_len + flash_start_addr, arr.size(), arr.data());
    dt = *((uint32_t *)arr.data());
    if (is_ok && can_use_dev) {
        log_1 << dt;
        log_1 << f_l << f_h;
        add(addr_start, arr);
        if (dt == 0xFFFFFFFF) {
            f_h = addr_start;
            f_h_date_time = dt;
        } else
        if (f_l_date_time < dt) {
            f_l = addr_start;
            f_l_date_time = dt;
        } else {
            f_h = addr_start;
            f_h_date_time = dt;
        }
        log_1 << f_l << f_h << f_l + (f_h - f_l)/2;
        if (addr_start != f_l + (f_h - f_l)/2) {
            emit signal_cont_readFlash(f_l + (f_h - f_l)/2, len);
        }
//        emit signal_persent_complated(progress_load());
//        //emit signal_readFlash(addr_start, arr);
//        addr_start += ln;
//        len -= ln;
//        emit signal_cont_readFlash(addr_start, len);
        // 1468922930 1021 ok
        // 1468912720 0  1468911590

        // 1468922864 169
        // 1468878584 568 1468854764
    }
}

void model_arch_search::add_record(uint timestamp, uint addr)
{
    layoutAboutToBeChanged();
    QVariantMap vmap;
    vmap.insert("timestamp", timestamp);
    vmap.insert("addr", addr);
    datalist.append(vmap);
    layoutChanged();
    log_1 << "datalist.count" << datalist.count();
}

void model_arch_search::connect_dev(bool can_use)
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

void model_arch_search::add(int addr, QByteArray data)
{
    if (!data.size()) {
        log_1 << "no data";
        return;
    }
    if (data.size() < 4) return;
    uint32_t *ptime = (uint32_t *)data.data();
    add_record(*ptime, addr);

    //flash_curr_addr = addr + data.size();
    //log_1 << "flash_curr_addr" << flash_curr_addr;
}

float model_arch_search::progress_load()
{
    return ((float)(flash_curr_addr - flash_start_addr)) / flash_len;
}

void model_arch_search::clear()
{
    layoutAboutToBeChanged();
    datalist.clear();
    can_use_dev = false;

    flash_curr_addr = flash_start_addr;
    log_1 << "flash_curr_addr" << flash_curr_addr;
    layoutChanged();
}
