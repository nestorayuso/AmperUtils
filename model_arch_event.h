#ifndef MODEL_ARCH_EVENT_H
#define MODEL_ARCH_EVENT_H

#include "model_arch_data.h"

class model_arch_event : public model_arch_data
{
    Q_OBJECT
public:
    explicit model_arch_event(amper_dev * dev, QObject *parent = 0);
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual void set_header(QByteArray data);
    virtual void add(QByteArray data);
};

#endif // MODEL_ARCH_EVENT_H
