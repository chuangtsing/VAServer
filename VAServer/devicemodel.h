#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H

#include <QAbstractTableModel>
#include "mobile_device.h"

class DeviceModel : public QAbstractTableModel
{
    Q_OBJECT
    
private:
    vanet::MobileDevice *dev;
public:
    DeviceModel(QObject *parent = 0, vanet::MobileDevice *dev = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                                     int role = Qt::DisplayRole) const;
    vanet::MobileDevice* device() { return dev; };


public slots:
};

#endif // DEVICEMODEL_H
