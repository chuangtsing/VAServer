#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <QThread>
#include "queryclass.h"
#include <QHeaderView>
#include "devicemodel.h"
#include <iostream>
#include <QDesktopServices>
#include <QUrl>
#include "configuration.h"

using namespace std;

extern Configuration *config;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);

    ui->classList->addItems(ResultsModel::classList);
    devices.clear();
    devices.push_back(make_pair(ui->dev1Label, ui->dev1List));
    devices.push_back(make_pair(ui->dev2Label, ui->dev2List));
    devices.push_back(make_pair(ui->dev3Label, ui->dev3List));
    devices.push_back(make_pair(ui->dev4Label, ui->dev4List));

    stackedWidgets.push_back(ui->stackedDev1);
    stackedWidgets.push_back(ui->stackedDev2);
    stackedWidgets.push_back(ui->stackedDev3);
    stackedWidgets.push_back(ui->stackedDev4);
}

MainWindow::~MainWindow()
{
    delete ui;
	delete resultsModelMatch;
	delete resultsModelNoMatch;
}

void MainWindow::on_btnQuery_released()
{
    //int[] selected = ui->classList->selectedIndexes();
    for(QStackedWidget *w : stackedWidgets)
        w->setCurrentIndex(0);
    ui->stackedWidget->setCurrentIndex(1);
    devCount = 0;
	fp = 0;
	fn = 0;
    QThread *t = new QThread;
    vanet::Scheduling sched;
    if (ui->radioButtonServer->isChecked())
        sched = vanet::SERVER;
    else if (ui->radioButtonMobile->isChecked())
        sched = vanet::MOBILE;
    else
        sched = vanet::OPT;


    resultsModelMatch = new ResultsModel();
    resultsModelNoMatch = new ResultsModel();
    ui->resultsListMatch->setModel(resultsModelMatch);
    ui->resultsListNoMatch->setModel(resultsModelNoMatch);
    ui->resultsListMatch->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->resultsListNoMatch->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->labelFN->setText("False negatives:");
    ui->labelFP->setText("False positives:");
    ui->labelElapsedTime->setText("Elapsed time:");

    connect(ui->resultsListMatch, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openVideo(QModelIndex)));
    connect(ui->resultsListNoMatch, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openVideo(QModelIndex)));
    QModelIndexList list = ui->classList->selectionModel()->selectedIndexes();
    selectedClasses.clear();
    for (const QModelIndex &index : list)
        selectedClasses.push_back(index.row());
    vanet::QueryClass *query = new vanet::QueryClass(0, this, sched, ui->spinBoxK->value(), selectedClasses);
    query->moveToThread(t);
    connect(t, SIGNAL(started()), query, SLOT(runQuery()));
    connect(query, SIGNAL(queryFinished()), this, SLOT(finish()));
    connect(query, SIGNAL(finished()), t, SLOT(quit()));
    connect(query, SIGNAL(finished()), query, SLOT(deleteLater()));
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));
    connect(query, SIGNAL(addDevice(vanet::MobileDevice*)), this, SLOT(addDeviceList(vanet::MobileDevice*)));
    connect(query, SIGNAL(updateVideo(Video*)), this, SLOT(updateVideoStatus(Video*)));
    connect(query, SIGNAL(updateCountLabels(int,int)), this, SLOT(updateCountLabels(int,int)));
    connect(query, SIGNAL(startTimer()), this, SLOT(startTimer()));
    t->start();

}

void MainWindow::finish() {
    clock_gettime(CLOCK_MONOTONIC, &stop);
    double elapsed = (stop.tv_sec - start.tv_sec);
    elapsed += (stop.tv_nsec - start.tv_nsec) / 1000000000.0;
    ui->labelElapsedTime->setText(QString("Elapsed time: ") + QString::number(elapsed) + QString(" s"));
}

void MainWindow::addDeviceList(vanet::MobileDevice *dev) {
    devices[devCount].first->setText(QString::fromStdString(dev->get_ip()));
    devices[devCount].second->setModel(new DeviceModel(this, dev));
    devices[devCount].second->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    stackedWidgets[devCount]->setCurrentIndex(1);
    dev->index = devCount++;
}

void MainWindow::updateVideoStatus(Video *vid) {
    if (vid->status == DONE) {
		Video *actual = config->getVideo(vid->name);
        if (vid->is_match(selectedClasses)) {
            bool correct = true;
            if (actual != NULL){
                if (!actual->is_match(selectedClasses)) {
                    correct = false;
                }
            }
            int i = resultsModelMatch->addVideo(vid, correct);
            QPoint q;
            q.setX(i);
            QModelIndex index = ui->resultsListMatch->indexAt(q);
            //ui->resultsListMatch->update(index);
            //ui->resultsListMatch->rowCountChanged(i, i+1);
            ui->resultsListMatch->viewport()->repaint();
        }
        else {
            bool correct = true;
            if (actual != NULL) {
                if (actual->is_match(selectedClasses)) {
                    correct = false;
                }
            }
            int i = resultsModelNoMatch->addVideo(vid, correct);
            QPoint q;
            q.setX(i);;
            QModelIndex index = ui->resultsListNoMatch->indexAt(q);
            //ui->resultsListNoMatch->update(index);
            //ui->resultsListNoMatch->rowCountChanged(i, i+1);
            ui->resultsListNoMatch->viewport()->repaint();
        }
    }
    QModelIndex i = devices[vid->dev->index].second->indexAt(QPoint(vid->index, 2));
    devices[vid->dev->index].second->update(i);
    devices[vid->dev->index].second->viewport()->repaint();
    ui->labelFP->setText(QString("False positives: ") + QString::number(resultsModelMatch->getIncorrect()));
    ui->labelFN->setText(QString("False negatives: ") + QString::number(resultsModelNoMatch->getIncorrect()));

    //repaint();
    //cout << "UPDATE!!!\n";
    //ui->listView->setModel();
}

void MainWindow::openVideo(const QModelIndex &index) {
    QTableView *table = (QTableView*) sender();
    Video *vid = ((ResultsModel*) table->model())->getVideo(index.row());
    QUrl url(QString::fromStdString(vid->local_path));
    QDesktopServices::openUrl(url);
}

void MainWindow::updateCountLabels(int server, int mobile) {
    ui->labelServer->setText(QString("Server: ") + QString::number(server));
    ui->labelMobile->setText(QString("Mobile: ") + QString::number(mobile));
}

void MainWindow::on_btnNewQuery_released()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::startTimer() {
    clock_gettime(CLOCK_MONOTONIC, &start);
    cout << "Timer started\n";
}
