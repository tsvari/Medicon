#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "TestSharedUtility.h"
#include "GrpcProxySortFilterModel.h"
#include "GrpcTemplateController.h"
#include "GrpcMasterSlaveController.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Master Template classes
    GrpcProxySortFilterModel * masterProxy = new GrpcProxySortFilterModel(
        new  GrpcTestObjectTableModel(ui->masterTableView),
                                    {0, 5, 6}, // Uid and Level Uid
                                    ui->masterTableView);
    MasterTemplate * masterTemplate = new MasterTemplate(masterProxy, ui->masterTableView, ui->masterForm, this);
    masterTemplate->addActionBars(this, ui->mainMenuBar, ui->mainToolBar, ui->mainStatusBAr);

    // Slave Template classes
    GrpcProxySortFilterModel * slaveProxy = new GrpcProxySortFilterModel(
        new GrpcTestSlaveObjectTableModel(ui->slaveTableView),
                                        {0, 1},
                                        ui->slaveTableView);
    SlaveTemplate * slaveTemplate = new SlaveTemplate(slaveProxy, ui->slaveTableView, ui->slaveForm, this);
    slaveTemplate->addActionBars(this, ui->mainMenuBar, ui->mainToolBar, ui->mainStatusBAr);

    // Controller
    GrpcMasterSlaveController * controller = new GrpcMasterSlaveController(masterTemplate, slaveTemplate, this);

    masterTemplate->addSearchForm(ui->searchForm);
    connect(ui->searchButton, &QPushButton::clicked, ui->searchForm, &GrpcSearchForm::submit);

    GrpcTestLevelObjectTableModel * comboModel = new  GrpcTestLevelObjectTableModel(std::move(TestModelData::comboLevelData()), ui->levelCombo);
    ui->levelCombo->setModel(comboModel);
    ui->levelCombo->setModelColumn(1);
    ui->levelCombo->setPlaceholderText("...");
    ui->levelCombo->setCurrentIndex(-1); // show nothing when start

    ui->masterTableView->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    //int row = ui->levelCombo->currentIndex();
    //if(GrpcObjectTableModel * model = qobject_cast<GrpcObjectTableModel*>(ui->levelCombo->model()); row > -1) {
    //    QString data = model->data(model->index(row, 0)).toString();
    //}

    //// Controller sends object with UID 5; find the row index and select combo
    //std::string objectUid = "5";
    //if(GrpcObjectTableModel * model = qobject_cast<GrpcObjectTableModel*>(ui->levelCombo->model())) {
    //    const QModelIndexList foundIndexList = model->match(model->index(0, 0), Qt::DisplayRole, FrontConverter::to_str(objectUid));
    //    if(foundIndexList.count() > 0) {
    //        ui->levelCombo->setCurrentIndex(foundIndexList.at(0).row());
    //    }
    //}

    //ui->masterForm->fillObject();

    //QVariant filledObject = ui->masterForm->object();
    //Q_ASSERT(filledObject.isValid());

    //GprcTestDataObject object = filledObject.value<GprcTestDataObject>();

}

