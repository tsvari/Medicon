#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "TestSharedUtility.h"
#include "GrpcProxySortFilterModel.h"
#include "GrpcUiTemplate.h"
#include "GrpcTemplateController.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Master Template classes
    GrpcProxySortFilterModel * masterProxy = new GrpcProxySortFilterModel(
        new  GrpcTestObjectTableModel(std::move(TestModelData::masterData()),
                                    ui->masterTableView),
                                    {0, 6}, // Uid and Level Uid
                                    ui->masterTableView);
    GrpcUiTemplate * masterTemplate = new GrpcUiTemplate(masterProxy, ui->masterTableView, ui->masterForm, this);

    // Slave Template classes
    GrpcProxySortFilterModel * slaveProxy = new GrpcProxySortFilterModel(
        new GrpcTestSlaveObjectTableModel(std::move(TestModelData::slaveData()),
                                    ui->slaveTableView),
                                    {0, 1},
                                    ui->slaveTableView);
    GrpcUiTemplate * slaveTemplate = new GrpcUiTemplate(slaveProxy, ui->slaveTableView, ui->slaveForm, this);

    // Controller
    GrpcTemplateController * controller = new GrpcTemplateController(masterTemplate, slaveTemplate, this);

    GrpcTestLevelObjectTableModel * comboModel = new  GrpcTestLevelObjectTableModel(std::move(TestModelData::comboLevelData()), ui->levelCombo);
    ui->levelCombo->setModel(comboModel);
    ui->levelCombo->setModelColumn(1);
    ui->levelCombo->setPlaceholderText("......");
    ui->levelCombo->setCurrentIndex(-1); // show nothing when start
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    int row = ui->levelCombo->currentIndex();
    if(GrpcObjectTableModel * model = qobject_cast<GrpcObjectTableModel*>(ui->levelCombo->model()); row > -1) {
        QString data = model->data(model->index(row, 0)).toString();
    }

    // Controller sends object with UID 5; find the row index and select combo
    std::string objectUid = "5";
    if(GrpcObjectTableModel * model = qobject_cast<GrpcObjectTableModel*>(ui->levelCombo->model())) {
        const QModelIndexList foundIndexList = model->match(model->index(0, 0), Qt::DisplayRole, FrontConverter::to_str(objectUid));
        if(foundIndexList.count() > 0) {
            ui->levelCombo->setCurrentIndex(foundIndexList.at(0).row());
        }
    }

}

