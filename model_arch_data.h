#ifndef MODEL_ARCH_DATA_H
#define MODEL_ARCH_DATA_H

#include <QAbstractTableModel>
#include <stdint.h>
#include "amper_dev.h"

class model_arch_data : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit model_arch_data(amper_dev * dev, QObject *parent = 0);
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual void set_header(QByteArray data);
    void clear();
    virtual void add(QByteArray data);
    void connect_dev(bool can_use);

    // todo
    uint flash_start_addr;
    uint flash_len;
    uint flash_curr_addr;
    QByteArray read_buff;
    virtual void add(int addr, QByteArray data);
    float progress_load();
protected:
    amper_dev * dev;
    volatile bool can_use_dev;
    volatile bool is_use_dev;

    QVariantList datalist;
    QVariant header_settings;
    uint communication_len;
    int record_len;

    uint8_t ARCH_crc8(const void *vptr, int len);

signals:
    void signal_persent_complated(float persent);
    //void signal_readFlash(int addr, QByteArray data);
    void signal_cont_readFlash(int addr_start, int len);
public slots:
    void slot_readFlash(int addr_start, int len);
};

#endif // MODEL_ARCH_DATA_H
