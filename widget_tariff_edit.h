#ifndef WIDGET_TARIFF_EDIT_H
#define WIDGET_TARIFF_EDIT_H

#include <QWidget>
#include "amper_dev.h"
#include <QProgressBar>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>

namespace Ui {
class widget_tariff_edit;
}

class model_tariff : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit model_tariff(QObject *parent = 0);
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual void set_data(void * tariff_data);
    virtual QByteArray get_data();
    virtual QString zone_to_string(const int zone) const;
protected:
    QVector<int> tariff_arr;
};

class tariff_delegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    tariff_delegate(QAbstractTableModel *model, QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
//    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
//    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const Q_DECL_OVERRIDE;
protected:
    QAbstractTableModel *model;
signals:
    void signal_end(const QModelIndex &index) const;
private slots:
    void commitAndCloseEditor(QModelIndex index);
};

class widget_tariff_edit : public QWidget
{
    Q_OBJECT

public:
//    explicit widget_tariff_edit(amper_dev *ad, QProgressBar *pb, QWidget *parent = 0);
    explicit widget_tariff_edit(QWidget *parent = 0);
    ~widget_tariff_edit();
    QByteArray getData();
private:
    Ui::widget_tariff_edit *ui;
//    amper_dev * dev;
//    QProgressBar *bar;
    model_tariff * model;
    void *ptariff;
public slots:
    void setData(void *ptariff);
//    void readAllData();
};

#endif // WIDGET_TARIFF_EDIT_H
