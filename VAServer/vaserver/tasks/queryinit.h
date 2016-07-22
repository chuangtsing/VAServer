#ifndef QUERYINIT_H
#define QUERYINIT_H

#include <QObject>
#include <string>
#include "queryclass.h"

namespace vanet {
class QueryInit : public QObject
{
    Q_OBJECT
public:
    explicit QueryInit(QObject *parent = 0, QueryClass *queryClassParent = 0,
                       std::string ip = "");

private:
    std::string ip;
    QueryClass *queryClassParent;

signals:
    void finished();
    void deviceAdded(MobileDevice *dev);

public slots:
    void run();
};
}
#endif // QUERYINIT_H
