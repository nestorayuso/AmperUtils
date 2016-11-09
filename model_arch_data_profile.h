#ifndef MODEL_ARCH_DATA_PROFILE_H
#define MODEL_ARCH_DATA_PROFILE_H

#include "model_arch_data.h"

class model_arch_data_profile : public model_arch_data
{
    Q_OBJECT
public:
    explicit model_arch_data_profile(amper_dev * dev, QObject *parent = 0);
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual void add(QByteArray data);
};

#endif // MODEL_ARCH_DATA_PROFILE_H
