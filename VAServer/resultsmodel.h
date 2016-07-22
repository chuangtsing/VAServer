#ifndef RESULTSMODEL_H
#define RESULTSMODEL_H

#include <QAbstractTableModel>
#include "mobile_device.h"


class ResultsModel : public QAbstractTableModel
{
    Q_OBJECT
    
private:
    std::vector<Video*> videos;
	std::vector<bool> videosCorrect;
    int correctCount = 0;
    int incorrectCount = 0;

public:
    ResultsModel(QObject *parent = 0, std::vector<Video*> videos = std::vector<Video*>());
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                                     int role = Qt::DisplayRole) const;
    int addVideo(Video *vid, bool correct);
    Video* getVideo(int index) { return videos[index]; };
    static const QStringList classList;
	int getIncorrect() { return incorrectCount; };



public slots:
};

#endif // RESULTSMODEL_H
