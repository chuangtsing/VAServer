#include "devicemodel.h"

using namespace vanet;
using namespace std;

DeviceModel::DeviceModel(QObject *parent, MobileDevice *dev) : QAbstractTableModel(parent)
{
    this->dev = dev;
}

int DeviceModel::rowCount(const QModelIndex &parent) const {
    return dev->get_videos().size();
}

int DeviceModel::columnCount(const QModelIndex &parent) const {
    return 3;
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        Video *vid = dev->get_videos()[index.row()];

        switch(index.column()) {
        case 0:
            return QString::fromStdString(vid->name);
        case 1:
            return QString("%1 KB").arg(vid->size / 1000);
        case 2:
            switch (vid->status) {
            case PENDING:
                return QString("Pending");
            case PROCESSING_MOBILE:
                return QString("Processing on mobile");
            case PROCESSING_SERVER:
                return QString("Processing on server");
            case UPLOADING:
                return QString("Uploading");
            case DONE:
                return QString("Done");
            }
        }
    }
    
    return QVariant();
}

QVariant DeviceModel::headerData(int section, Qt::Orientation orientation,
                                 int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        switch(section) {
        case 0:
            return QString("Name");
        case 1:
            return QString("Size");
        case 2:
            return QString("Status");
        }
    }

    return QVariant();
}
