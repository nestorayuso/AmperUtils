#include "model_arch_event.h"

#include <QDateTime>
#include <QTimeZone>

#include "meter.h"
#include "logger.h"

Q_DECLARE_METATYPE(METER_ARCH_EVENT);

model_arch_event::model_arch_event(amper_dev * dev, QObject *parent)
    : model_arch_data(dev, parent)
{
    record_len = (sizeof(METER_ARCH_EVENT));
}

int model_arch_event::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant model_arch_event::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole: {
        if (index.row() < 0 || index.row() >= datalist.count()) break;
        METER_ARCH_EVENT mae = datalist.value(index.row()).value<METER_ARCH_EVENT>();
        switch (index.column()) {
        //case 0: return mad.timestamp;//QDateTime::fromTime_t(mad.timestamp, QTimeZone(0)).toString("yyyy.MM.dd hh.mm");
        case 0: return QDateTime::fromTime_t(mae.timestamp, QTimeZone(0)).toString("yyyy.MM.dd hh.mm");
        case 1: {
            switch ((int)mae.event) {
            case ME_update_fw:      return tr("обновление прошивки");
            case ME_back_fw:        return tr("возврат к заводской прошивки");

            case ME_update_rts:     return tr("установка времени на");
            case ME_update_rts_old: return tr("перевод времени с");

            case ME_tariff_new:     return tr("смена тарифного расписания");
            case ME_k_new:          return tr("смена коэффициента трансформации");
            case ME_quality_set_new:return tr("смена параметров качества");

            case ME_btn_cover_up:   return tr("вскрытие клемной крышки");
            case ME_btn_cover_down: return tr("закрытие клемной крышки");
            case ME_btn_body_up:    return tr("вскрытие корпуса");
            case ME_btn_body_down:  return tr("закрытие корпуса");
            case ME_over_magnetic_field_start:  return tr("начало воздействия сверхнормативного магнитного воздействия");
            case ME_over_magnetic_field_stop:   return tr("окончание воздействия сверхнормативного магнитного воздействия");

            case ME_over_load:      return tr("отключение реле - разрыв цепи");
            case ME_under_load:     return tr("включение реле - замыкание цепи");

            case ME_clear_values:       return tr("сброс показаний (интеграторов)");
            case ME_clear_arch_data1:   return tr("очистка журнала данных 1");
            case ME_clear_arch_data2:   return tr("очистка журнала данных 2");
            case ME_clear_arch_event:   return tr("очистка журнала событий");

            case ME_set_arch_data1: return tr("новые настройки для журнала данных 1");
            case ME_set_arch_data2: return tr("новые настройки для журнала данных 2");

            case ME_max_U1_start:   return tr("начало превышения максимума напряжения фазы 1");
            case ME_max_U1_stop:    return tr("завершение превышения максимума напряжения фазы 1");
            case ME_min_U1_start:   return tr("начало превышения минимума  напряжения фазы 1");
            case ME_min_U1_stop:    return tr("завершение превышения минимума  напряжения фазы 1");
            case ME_max_U2_start:   return tr("начало превышения максимума напряжения фазы 2");
            case ME_max_U2_stop:    return tr("завершение превышения максимума напряжения фазы 2");
            case ME_min_U2_start:   return tr("начало превышения минимума  напряжения фазы 2");
            case ME_min_U2_stop:    return tr("завершение превышения минимума  напряжения фазы 2");
            case ME_max_U3_start:   return tr("начало превышения максимума напряжения фазы 3");
            case ME_max_U3_stop:    return tr("завершение превышения максимума напряжения фазы 3");
            case ME_min_U3_start:   return tr("начало превышения минимума  напряжения фазы 3");
            case ME_min_U3_stop:    return tr("завершение превышения минимума  напряжения фазы 3");
            case ME_max_F_start:    return tr("начало превышения максимума частоты");
            case ME_max_F_stop:     return tr("завершение превышения максимума частоты");
            case ME_min_F_start:    return tr("начало превышения минимума  частоты");
            case ME_min_F_stop:     return tr("завершение превышения минимума  частоты");

            case ME_power_on:       return tr("включение счетчика");
            case ME_power_off:      return tr("отключение счетчика");
            case ME_power_up_f1:    return tr("подключена фаза 1");
            case ME_power_down_f1:  return tr("отключена фаза 1");
            case ME_power_up_f2:    return tr("подключена фаза 2");
            case ME_power_down_f2:  return tr("отключена фаза 2");
            case ME_power_up_f3:    return tr("подключена фаза 3");
            case ME_power_down_f3:  return tr("отключена фаза 3");

            case ME_direction_I1:   return tr("смена направления тока фазы 1");
            case ME_direction_I2:   return tr("смена направления тока фазы 2");
            case ME_direction_I3:   return tr("смена направления тока фазы 3");

            default: return mae.event;
            }
            break;
        }
        case 2: {
            uint16_t mae_data;// = *((uint16_t*)&mae.reserve);
            memmove(&mae_data, &mae.reserve[0], sizeof(uint16_t));
            return QString::number(mae_data);
        }
        default: break;
        }
        break;
    }
    case Qt::TextAlignmentRole: return Qt::AlignRight + Qt::AlignVCenter; //QAbstractTableModel::headerData(section, orientation, role);
    default: break;
    }
    return QVariant();
    //return model_arch_data::data(index, role);
}

QVariant model_arch_event::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) return QVariant();
    switch (role) {
    case Qt::DisplayRole: {
        switch (section) {
        case 0: return tr("Время события");
        case 1: return tr("Событие");
        case 2: return tr("Параметр");
        default: break;
        }
        break;
    }
    default: break;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void model_arch_event::set_header(QByteArray data)
{
    Q_UNUSED(data);
}

void model_arch_event::add(QByteArray data)
{
    if (data.size() != record_len) return;
    METER_ARCH_EVENT * mae = (METER_ARCH_EVENT*)data.data();
    if (ARCH_crc8(mae, record_len - 1) != mae->crc) {
        //log_1 << "error crc" << mae->crc << ARCH_crc8(mae, record_len - 1) << data.toHex();
        return;
    }
    if (!mae->timestamp) {
        return;
    }
    layoutAboutToBeChanged();
    datalist.append(QVariant::fromValue(*mae));
    layoutChanged();
    log_1 << "datalist.count" << datalist.count() << data.toHex().toUpper();

}
