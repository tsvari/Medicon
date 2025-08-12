#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "TestSharedUtility.h"
#include "GrpcProxySortFilterModel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    GrpcProxySortFilterModel * masterProxy = new GrpcProxySortFilterModel(
        new  GrpcTestObjectTableModel(std::move(TestModelData::masterData()), ui->masterTableView),
        {0}, ui->masterTableView);
    ui->masterTableView->setModel(masterProxy);

    GrpcProxySortFilterModel * slaveProxy = new GrpcProxySortFilterModel(
        new GrpcTestSlaveObjectTableModel(std::move(TestModelData::slaveData()), ui->slaveTableView),
        {0, 1}, ui->slaveTableView);
    ui->slaveTableView->setModel(slaveProxy);

}

MainWindow::~MainWindow()
{
    delete ui;
}
