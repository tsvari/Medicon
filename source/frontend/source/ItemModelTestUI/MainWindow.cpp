#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "TestSharedUtility.h"
#include "GrpcProxySortFilterModel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    GrpcTestObjectTableModel * masterModel = new  GrpcTestObjectTableModel(std::move(TestModelData::masterData()), ui->masterTableView);
    ui->masterTableView->setModel(masterModel);

    GrpcTestSlaveObjectTableModel * slaveModel = new GrpcTestSlaveObjectTableModel(std::move(TestModelData::slaveData()), ui->slaveTableView);
    ui->slaveTableView->setModel(slaveModel);
}

MainWindow::~MainWindow()
{
    delete ui;
}
