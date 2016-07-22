#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include "mobile_device.h"
#include <QTableView>
#include <QLabel>
#include <QStackedWidget>
#include <utility>
#include "resultsmodel.h"
#include <ctime>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    const QStringList &getClassList() { return classList; };

private:
    int devCount;
    Ui::MainWindow *ui;
    std::vector<QStackedWidget*> stackedWidgets;
    std::vector<std::pair<QLabel*, QTableView*>> devices;
    ResultsModel *resultsModelMatch;
    ResultsModel *resultsModelNoMatch;
    std::vector<int> selectedClasses;
    QStringList classList;

    struct timespec start, stop;

	int fp;
	int fn;



public slots:
    //void clearDevices();
    void addDeviceList(vanet::MobileDevice*);
    void updateVideoStatus(Video *vid);
    void updateCountLabels(int server, int mobile);
    void startTimer();

private slots:
    void on_btnQuery_released();
    void finish();
    void openVideo(const QModelIndex &index);

    void on_btnNewQuery_released();

signals:

};

#endif // MAINWINDOW_H
