#include <gtest/gtest.h>

#include "GrpcMasterSlaveController.h"

#include "GrpcTemplateController.h"
#include "GrpcProxySortFilterModel.h"
#include "GrpcTableView.h"

#include "GrpcDataContainer.hpp"
#include "GrpcForm.h"
#include "GrpcObjectTableModel.h"
#include "GrpcObjectWrapper.hpp"
#include "TestSharedUtility.h"

#include <QApplication>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QTest>
#include <QToolBar>

#include <functional>
#include <memory>

namespace {

void processEventsUntil(const std::function<bool()> & predicate, int timeoutMs = 1500)
{
    QElapsedTimer timer;
    timer.start();

    while (!predicate()) {
        if (timer.elapsed() > timeoutMs) {
            break;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 25);
        QTest::qWait(5);
    }
}

class TestGrpcObjectTableModel : public GrpcObjectTableModel
{
public:
    explicit TestGrpcObjectTableModel(QObject * parent = nullptr)
        : GrpcObjectTableModel(new GrpcDataContainer<MasterObject>(), parent)
    {
        initializeModel();
        initializeContainer();
    }

    void initializeModel() override
    {
        auto * container = dynamic_cast<GrpcDataContainer<MasterObject> *>(objectContainer());
        if (!container) {
            return;
        }

        container->addProperty("Uid", DataInfo::Int, &MasterObject::set_uid, &MasterObject::uid);
        container->addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    }
};

class MasterSlaveTestForm : public GrpcForm
{
public:
    explicit MasterSlaveTestForm(QWidget * parent = nullptr)
        : GrpcForm(new GrpcObjectWrapper<MasterObject>(), nullptr, parent)
    {
        // The controller will call initializeForm() and initilizeWidgets() as a friend.
    }

    void initializeForm() override
    {
        auto * wrapper = dynamic_cast<GrpcObjectWrapper<MasterObject> *>(objectWrapper());
        if (!wrapper) {
            return;
        }

        // Create widgets with object names that match wrapper properties.
        m_nameEdit = new QLineEdit(this);
        m_nameEdit->setObjectName("Name");

        wrapper->addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    }

    QVariant defaultObject() override
    {
        MasterObject obj;
        obj.set_uid(0);
        obj.set_name("");
        return QVariant::fromValue(obj);
    }

private:
    QLineEdit * m_nameEdit = nullptr;
};

class MasterSlaveTestTemplateController : public GrpcTemplateController
{
public:
    explicit MasterSlaveTestTemplateController(GrpcProxySortFilterModel * proxyModel,
                                               GrpcTableView * tableView,
                                               GrpcForm * form,
                                               QObject * parent = nullptr)
        : GrpcTemplateController(proxyModel, tableView, form, nullptr, parent)
    {
    }

    void emitRowChangedForTest(const QModelIndex & index)
    {
        emit rowChanged(index);
    }

    bool menuVisible()
    {
        if (!templateMenu()) {
            return false;
        }
        return templateMenu()->menuAction()->isVisible();
    }

    bool toolbarVisible()
    {
        if (!templateToolBar()) {
            return false;
        }
        return !templateToolBar()->isHidden();
    }

protected:
    void workerModelData() override
    {
        // Not needed for these tests.
        auto container = std::make_shared<GrpcDataContainer<MasterObject>>(std::vector<MasterObject>{});
        QMetaObject::invokeMethod(this, [this, container]() {
            emit populateModel(container);
        }, Qt::QueuedConnection);
    }

    QVariant workerAddNewObject(const QVariant & promise) override { return promise; }
    QVariant workerEditObject(const QVariant & promise) override { return promise; }
    QVariant workerDeleteObject(const QVariant & promise) override { return promise; }
};

struct ControllerBundle {
    std::unique_ptr<QToolBar> baseToolBar;
    std::unique_ptr<TestGrpcObjectTableModel> model;
    std::unique_ptr<GrpcProxySortFilterModel> proxy;
    std::unique_ptr<GrpcTableView> view;
    std::unique_ptr<MasterSlaveTestForm> form;
    std::unique_ptr<MasterSlaveTestTemplateController> controller;
};

ControllerBundle createController(QMainWindow * mainWindow)
{
    ControllerBundle b;

    b.baseToolBar = std::make_unique<QToolBar>(mainWindow);
    mainWindow->addToolBar(b.baseToolBar.get());

    b.model = std::make_unique<TestGrpcObjectTableModel>(mainWindow);
    b.proxy = std::make_unique<GrpcProxySortFilterModel>(b.model.get(), QList<int>{}, mainWindow);
    b.view = std::make_unique<GrpcTableView>(mainWindow);
    b.form = std::make_unique<MasterSlaveTestForm>(mainWindow);

    b.controller = std::make_unique<MasterSlaveTestTemplateController>(b.proxy.get(), b.view.get(), b.form.get(), mainWindow);

    // Avoid modal dialogs during tests.
    QObject::disconnect(b.controller.get(), &GrpcTemplateController::warning, b.view.get(), &GrpcTableView::showWarning);

    b.controller->addActionBars(mainWindow, mainWindow->menuBar(), b.baseToolBar.get(), mainWindow->statusBar());

    return b;
}

class GrpcMasterSlaveControllerFixture : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        if (!qApp) {
            int argc = 0;
            char * argv[] = { nullptr };
            s_app = new QApplication(argc, argv);
        }
    }

    void SetUp() override
    {
        mainWindow = std::make_unique<QMainWindow>();

        master = createController(mainWindow.get());
        slave = createController(mainWindow.get());

        msController = std::make_unique<GrpcMasterSlaveController>(master.controller.get(), slave.controller.get(), mainWindow.get());

        QCoreApplication::processEvents();
    }

    static QApplication * s_app;

    std::unique_ptr<QMainWindow> mainWindow;
    ControllerBundle master;
    ControllerBundle slave;
    std::unique_ptr<GrpcMasterSlaveController> msController;
};

QApplication * GrpcMasterSlaveControllerFixture::s_app = nullptr;

} // namespace

TEST_F(GrpcMasterSlaveControllerFixture, MasterRowChangedPropagatesToSlaveMasterRowChanged)
{
    QStandardItemModel dummy;
    dummy.setRowCount(1);
    dummy.setColumnCount(1);
    QModelIndex index = dummy.index(0, 0);

    QSignalSpy spy(slave.controller.get(), SIGNAL(masterRowChanged(QModelIndex)));

    master.controller->emitRowChangedForTest(index);
    QCoreApplication::processEvents();

    ASSERT_EQ(spy.count(), 1);
    const QModelIndex received = spy.takeFirst().at(0).value<QModelIndex>();
    EXPECT_TRUE(received.isValid());
    EXPECT_EQ(received.row(), index.row());
    EXPECT_EQ(received.column(), index.column());
}

TEST_F(GrpcMasterSlaveControllerFixture, ClearAllClearsMasterSelectionAndPropagatesToSlave)
{
    QSignalSpy spy(slave.controller.get(), SIGNAL(masterRowChanged(QModelIndex)));

    msController->clearAll();
    QCoreApplication::processEvents();

    ASSERT_GE(spy.count(), 1);
    const QModelIndex received = spy.takeFirst().at(0).value<QModelIndex>();
    EXPECT_FALSE(received.isValid());
}

TEST_F(GrpcMasterSlaveControllerFixture, ShowingMasterMenuHidesSlaveMenu)
{
    EXPECT_FALSE(master.controller->menuVisible());
    EXPECT_FALSE(slave.controller->menuVisible());

    master.controller->showMenuAndToolbar();

    processEventsUntil([this]() {
        return master.controller->menuVisible();
    });

    EXPECT_TRUE(master.controller->menuVisible());
    EXPECT_TRUE(master.controller->toolbarVisible());

    EXPECT_FALSE(slave.controller->menuVisible());
    EXPECT_FALSE(slave.controller->toolbarVisible());
}

TEST_F(GrpcMasterSlaveControllerFixture, ShowingSlaveMenuHidesMasterMenu)
{
    slave.controller->showMenuAndToolbar();

    processEventsUntil([this]() {
        return slave.controller->menuVisible();
    });

    EXPECT_TRUE(slave.controller->menuVisible());
    EXPECT_TRUE(slave.controller->toolbarVisible());

    EXPECT_FALSE(master.controller->menuVisible());
    EXPECT_FALSE(master.controller->toolbarVisible());
}
