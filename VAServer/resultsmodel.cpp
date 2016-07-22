#include "resultsmodel.h"
#include <QColor>

using namespace vanet;
using namespace std;

const QStringList ResultsModel::classList = QStringList() <<"Person" << "Soldier" << "Army Tank" <<
        "Military Vehicle" << "Truck" << "Car" << "Launcher" << "Rifle" << "Explosion" << "Ship" <<
        "Helicopter" << "Animal" << "Background";

ResultsModel::ResultsModel(QObject *parent, vector<Video*> videos) : QAbstractTableModel(parent)
{
    this->videos = videos;
}

int ResultsModel::rowCount(const QModelIndex &parent) const {
    return videos.size();
}

int ResultsModel::columnCount(const QModelIndex &parent) const {
    return 3;
}

QVariant ResultsModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        Video *vid = videos[index.row()];

        switch(index.column()) {
        case 0:
            return QString::fromStdString(vid->name);
        case 1:
            return QString("%1 KB").arg(vid->size / 1000);
        case 2:
            switch (vid->mode) {
            case vanet_pb::ProcessMode::SERVER:
                return QString("Server");
            case vanet_pb::ProcessMode::MOBILE:
                return QString("Mobile");
            }
        }
    }
    else if (role == Qt::ToolTipRole) {
        Video *vid = videos[index.row()];
        QString ip = QString::fromStdString(vid->dev->get_ip());
        QString duration = QString::number((double) vid->duration / 1000.) + "s";
        QString classes;
        for (int i= 0; i < vid->get_tags().size() - 1; i++) {
            classes.append(QString::number(i+1) + ") " + classList[vid->get_tags()[i]] + "\n");
        }
        classes.append(QString::number(vid->get_tags().size()) + ") " + classList[vid->get_tags()[vid->get_tags().size() - 1]]);

        return QString("Device: %1\nDuration: %2\nClasses:\n%3").arg(ip, duration, classes);
    }
	else if(role == Qt::ForegroundRole) {
		if (videosCorrect[index.row()]) {
			return QColor(Qt::black);
		}
        return QColor(Qt::red);
	}
    
    return QVariant();
}

QVariant ResultsModel::headerData(int section, Qt::Orientation orientation,
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
            return QString("Process location");
        }
    }

    return QVariant();
}

int ResultsModel::addVideo(Video *vid, bool correct) {
    beginInsertRows(QModelIndex(), videos.size(), videos.size());
    videos.push_back(vid);
	videosCorrect.push_back(correct);
	if (correct)
		correctCount++;
	else
		incorrectCount++;
    endInsertRows();
    return videos.size() - 1;
}
