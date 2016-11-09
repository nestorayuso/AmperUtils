#include "widget_tariff_edit.h"
#include "ui_widget_tariff_edit.h"

#include <QTimer>
#include <QDateTime>
#include <QTimeZone>

#include "tariff.h"
#include "meter.h"
#include "logger.h"

widget_tariff_edit::widget_tariff_edit(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::widget_tariff_edit)
//    , dev(ad)
//    , bar(pb)
    , ptariff(0)
{
    ui->setupUi(this);
    ptariff = new TARIFF_TABLE;
    model = new model_tariff(this);
    ui->tableView->setModel(model);
    ui->tableView->setItemDelegate(new tariff_delegate(model, this));
    ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

widget_tariff_edit::~widget_tariff_edit()
{
    delete (TARIFF_TABLE*)ptariff;
    delete ui;
}

QByteArray widget_tariff_edit::getData()
{
    return model->get_data();
}

void widget_tariff_edit::setData(void *ptariff)
{
    model->set_data(ptariff);
}

//void widget_tariff_edit::readAllData()
//{
//    if(!dev->isReady())
//    {
//        QTimer::singleShot(1, this, SLOT(readAllData()));
//        return;
//    }
//    bar->setValue(50);

//    DTI_Property prop;
//    if(!dev->getPropertyByName("pTariff", &prop)) {
////        QMessageBox::warning(this,"Ошибка чтения",dev->error_message());
//        bar->setValue(0);
//        return;
//    }
//    if(!dev->readData(dev->propertySeg(&prop),prop.address, sizeof(TARIFF_TABLE), ptariff)) {
////        QMessageBox::warning(this,"Ошибка чтения",dev->error_message());
//        bar->setValue(0);
//        return;
//    }

//    model->set_data(ptariff);
////    TARIFF_TABLE * tt = (TARIFF_TABLE *)ptariff;
////    for (int i = 0; i < TARIFF_TABLE_ITEMS; i++) {
////        setZoneButton((uint8_t)(i*4)+0, tt->h2[i].hh0);
////        setZoneButton((uint8_t)(i*4)+1, tt->h2[i].hh1);
////        setZoneButton((uint8_t)(i*4)+2, tt->h2[i].hh2);
////        setZoneButton((uint8_t)(i*4)+3, tt->h2[i].hh3);
////    }

//    bar->setValue(100);
//    //QThread::msleep(100);
//    //bar->setValue(0);
//}

model_tariff::model_tariff(QObject *parent)
    : QAbstractTableModel(parent)
{
    tariff_arr.resize(TARIFF_ZONE_MAX);
}

int model_tariff::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return (TARIFF_ZONE_MAX >> 1);
}

int model_tariff::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return (TARIFF_MAX - 1) * 2 + 1 * 2;
}

QVariant model_tariff::data(const QModelIndex &index, int role) const
{
    int zone  = index.row();
    if (index.column() >= TARIFF_MAX) {
        zone += TARIFF_ZONE_MAX >> 1;
    }
    int tariff = tariff_arr[zone];
    int tariff_index = index.column() - 1;
    while (tariff_index >= TARIFF_MAX) {
        tariff_index -= TARIFF_MAX;
    }

    switch (role) {
    case Qt::DisplayRole: {
        if (index.column() == 0 || index.column() == TARIFF_MAX) {
            return zone_to_string(zone);
        }
        //return QString("%1 %2").arg(tariff_index).arg(zone);
        //if (tariff_arr[zone])
        break;
    }
    case Qt::ToolTipRole: break;
    case Qt::StatusTipRole: break;
    case Qt::DecorationRole: break;
    case Qt::FontRole: break;
    case Qt::TextAlignmentRole: break;
    case Qt::ForegroundRole: break; // QBrush
    case Qt::BackgroundColorRole: {
        if (tariff == tariff_index) {
            return QBrush(QColor(0, 255, 0));
        }
        break;
    }
    case Qt::CheckStateRole: break;
    case Qt::SizeHintRole: break;
    default: {
        log_1 << role;
        break;
    }
    }

    return QVariant();
}

QVariant model_tariff::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        int tariff_index = section - 1;
        while (tariff_index >= TARIFF_MAX) {
            tariff_index -= TARIFF_MAX;
        }
        tariff_index++;
        //return tariff_index;
        if (tariff_index <= 0) return "";
        if (tariff_index >= TARIFF_MAX) return "";
        return QString("T%1").arg(tariff_index);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

bool model_tariff::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(role);
    Q_UNUSED(value);
    int zone  = index.row();
    if (index.column() >= TARIFF_MAX) {
        zone += TARIFF_ZONE_MAX >> 1;
    }
//    int tariff = tariff_arr[zone];
    int tariff_index = index.column() - 1;
    while (tariff_index >= TARIFF_MAX) {
        tariff_index -= TARIFF_MAX;
    }
    layoutAboutToBeChanged();
    tariff_arr[zone] = tariff_index;
    layoutChanged();
    return true;
}

Qt::ItemFlags model_tariff::flags(const QModelIndex &index) const
{
    if (index.column() == 0 || index.column() == TARIFF_MAX) {
        return Qt::ItemIsEnabled;
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}

void model_tariff::set_data(void *tariff_data)
{
    layoutAboutToBeChanged();
    int zone = 0;
    TARIFF_TABLE * tt = (TARIFF_TABLE *)tariff_data;
    for (int i = 0; i < TARIFF_TABLE_ITEMS; i++) {
        tariff_arr[zone++] = tt->h2[i].hh0;
        tariff_arr[zone++] = tt->h2[i].hh1;
        tariff_arr[zone++] = tt->h2[i].hh2;
        tariff_arr[zone++] = tt->h2[i].hh3;
    }
    layoutChanged();
}

QByteArray model_tariff::get_data()
{
    QByteArray arr(tariff_arr.count() / 4, (char)0);
    for (int zone = 0; zone < tariff_arr.count(); ++zone) {
        TARIFF_TABLE * tt = (TARIFF_TABLE *)arr.data();
        int tti = zone / 4;
        switch (zone % 4) {
        case 0: tt->h2[tti].hh0 = tariff_arr[zone]; break;
        case 1: tt->h2[tti].hh1 = tariff_arr[zone]; break;
        case 2: tt->h2[tti].hh2 = tariff_arr[zone]; break;
        case 3: tt->h2[tti].hh3 = tariff_arr[zone]; break;
        default: break;
        }
    }
    return arr;
}

QString model_tariff::zone_to_string(const int zone) const
{
    return QDateTime::fromTime_t((zone + 0) * 60 * 30).toTimeZone(QTimeZone(0)).toString("hh.mm")
          +" - "
          +QDateTime::fromTime_t((zone + 1) * 60 * 30).toTimeZone(QTimeZone(0)).toString("hh.mm");
}

tariff_delegate::tariff_delegate(QAbstractTableModel *model, QObject *parent)
    : QStyledItemDelegate(parent)
    , model(model)
{
    connect(this, SIGNAL(signal_end(QModelIndex)), this, SLOT(commitAndCloseEditor(QModelIndex)));
}

void tariff_delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::paint(painter, option, index);
}

QSize tariff_delegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *tariff_delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(parent);
    Q_UNUSED(option);
    emit signal_end(index);
    return 0;
}

void tariff_delegate::commitAndCloseEditor(QModelIndex index)
{
    model->setData(index, 1);
}
