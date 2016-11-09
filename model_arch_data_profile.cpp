#include "model_arch_data_profile.h"

#include <QDateTime>
#include <QTimeZone>

#include "meter.h"
#include "logger.h"

Q_DECLARE_METATYPE(METER_ARCH_PROFILE_DATA);
Q_DECLARE_METATYPE(METER_ARCH_DATA_SETTINGS);

model_arch_data_profile::model_arch_data_profile(amper_dev *dev, QObject *parent)
    : model_arch_data(dev, parent)
{
    qRegisterMetaType<METER_ARCH_PROFILE_DATA>("METER_ARCH_PROFILE_DATA");
    record_len = (sizeof(METER_ARCH_PROFILE_DATA));
}

int model_arch_data_profile::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return ARCH_DATA_PROFILE_PARAM_COUT_MAX + 1;
}

QVariant model_arch_data_profile::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole: {
        if (index.row() < 0 || index.row() >= datalist.count()) break;
        METER_ARCH_PROFILE_DATA mad = datalist.value(index.row()).value<METER_ARCH_PROFILE_DATA>();
        switch (index.column()) {
        //case 0: return mad.timestamp;//QDateTime::fromTime_t(mad.timestamp, QTimeZone(0)).toString("yyyy.MM.dd hh.mm");
        case 0: return QDateTime::fromTime_t(mad.timestamp, QTimeZone(0)).toString("yyyy.MM.dd hh.mm");
        default: {
            METER_ARCH_DATA_SETTINGS mads = header_settings.value<METER_ARCH_DATA_SETTINGS>();
            METER_DATA_TYPE mdt = (METER_DATA_TYPE)mads.param_id[index.column() - 1].MDT;
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
                return mad.value[index.column()-1].f32 / 1000.0;
//                return QString("%1").arg(((double)mad.value[index.column()-1].u32) / 2000.0, 0, 'f', 3, QChar('0')) ;
            }
            case MDT_U      :
            case MDT_Ulin   :
            case MDT_I      :
            case MDT_F      :
            case MDT_cosFi  : {
                return mad.value[index.column()-1].f32;
            }
            default: break;
            }
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

void model_arch_data_profile::add(QByteArray data)
{
    if (data.size() != record_len) return;
    METER_ARCH_PROFILE_DATA * mad = (METER_ARCH_PROFILE_DATA*)data.data();
    //log_1 << data.toHex();
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
