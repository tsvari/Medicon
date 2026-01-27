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
#include <QStatusBar>
#include <QTest>
#include <QToolBar>

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

namespace {

void processEventsUntil(const std::function<bool()> & predicate, int timeoutMs = 2000)
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

// ---------------- Models ----------------

class MasterModel : public GrpcObjectTableModel
{
public:
    explicit MasterModel(QObject * parent = nullptr)
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

class SlaveModel : public GrpcObjectTableModel
{
public:
    explicit SlaveModel(QObject * parent = nullptr)
        : GrpcObjectTableModel(new GrpcDataContainer<SlaveObject>(), parent)
    {
        initializeModel();
        initializeContainer();
    }

    void initializeModel() override
    {
        auto * container = dynamic_cast<GrpcDataContainer<SlaveObject> *>(objectContainer());
        if (!container) {
            return;
        }

        container->addProperty("Uid", DataInfo::Int, &SlaveObject::set_uid, &SlaveObject::uid);
        container->addProperty("LinkUid", DataInfo::Int, &SlaveObject::set_link_uid, &SlaveObject::link_uid);
        container->addProperty("Phone", DataInfo::String, &SlaveObject::set_phone, &SlaveObject::phone);
    }
};

// ---------------- Forms ----------------

class MasterForm : public GrpcForm
{
public:
    explicit MasterForm(QWidget * parent = nullptr)
        : GrpcForm(new GrpcObjectWrapper<MasterObject>(), nullptr, parent)
    {
        // controller will call initializeForm() + initilizeWidgets()
    }

    void initializeForm() override
    {
        auto * wrapper = dynamic_cast<GrpcObjectWrapper<MasterObject> *>(objectWrapper());
        if (!wrapper) {
            return;
        }

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

class SlaveForm : public GrpcForm
{
public:
    explicit SlaveForm(QWidget * parent = nullptr)
        : GrpcForm(new GrpcObjectWrapper<SlaveObject>(), new GrpcObjectWrapper<MasterObject>(), parent)
    {
        // controller will call initializeForm() + initilizeWidgets()
    }

    void initializeForm() override
    {
        auto * wrapper = dynamic_cast<GrpcObjectWrapper<SlaveObject> *>(objectWrapper());
        if (!wrapper) {
            return;
        }

        m_phoneEdit = new QLineEdit(this);
        m_phoneEdit->setObjectName("Phone");

        wrapper->addProperty("Phone", DataInfo::String, &SlaveObject::set_phone, &SlaveObject::phone);
    }

    QVariant defaultObject() override
    {
        QVariant masterVarObject = masterVariantObject();
        Q_ASSERT(masterVarObject.isValid());

        MasterObject masterObject = masterVarObject.value<MasterObject>();

        SlaveObject obj;
        obj.set_uid(0);
        obj.set_link_uid(masterObject.uid());
        obj.set_phone("");
        return QVariant::fromValue(obj);
    }

    QLineEdit * phoneEdit() const { return m_phoneEdit; }

private:
    QLineEdit * m_phoneEdit = nullptr;
};

// ---------------- Controllers ----------------

class MasterTemplateController : public GrpcTemplateController
{
public:
    explicit MasterTemplateController(GrpcProxySortFilterModel * proxyModel,
                                      GrpcTableView * tableView,
                                      GrpcForm * form,
                                      QObject * parent = nullptr)
        : GrpcTemplateController(proxyModel, tableView, form, nullptr, parent)
    {
    }

    void setData(std::vector<MasterObject> data)
    {
        m_masterData = std::move(data);
    }

protected:
    void workerModelData() override
    {
        auto dataCopy = m_masterData;
        auto container = std::make_shared<GrpcDataContainer<MasterObject>>(std::move(dataCopy));
        // Ensure populateModel is emitted on this object's thread.
        QMetaObject::invokeMethod(this, [this, container]() {
            emit populateModel(container);
        }, Qt::QueuedConnection);
    }

    QVariant workerAddNewObject(const QVariant & promise) override
    {
        MasterObject obj = promise.value<MasterObject>();
        if (obj.uid() <= 0) {
            obj.set_uid(100);
        }
        if (obj.name().empty()) {
            obj.set_name("New");
        }
        return QVariant::fromValue(obj);
    }

    QVariant workerEditObject(const QVariant & promise) override { return promise; }
    QVariant workerDeleteObject(const QVariant & promise) override { return promise; }

private:
    std::vector<MasterObject> m_masterData;
};

class SlaveTemplateController : public GrpcTemplateController
{
public:
    explicit SlaveTemplateController(GrpcProxySortFilterModel * proxyModel,
                                     GrpcTableView * tableView,
                                     GrpcForm * form,
                                     QObject * parent = nullptr)
        : GrpcTemplateController(proxyModel, tableView, form, new GrpcObjectWrapper<MasterObject>(), parent)
    {
    }

    void setData(std::vector<SlaveObject> data)
    {
        m_slaveData = std::move(data);
    }

protected:
    void workerModelData() override
    {
        std::vector<SlaveObject> filtered;

        if (masterValid()) {
            const MasterObject masterObj = masterVariantObject().value<MasterObject>();
            std::copy_if(m_slaveData.begin(), m_slaveData.end(), std::back_inserter(filtered),
                         [masterObj](const SlaveObject & s) {
                             return s.link_uid() == masterObj.uid();
                         });
        }

        auto container = std::make_shared<GrpcDataContainer<SlaveObject>>(std::move(filtered));
        QMetaObject::invokeMethod(this, [this, container]() {
            emit populateModel(container);
        }, Qt::QueuedConnection);
    }

    QVariant workerAddNewObject(const QVariant & promise) override
    {
        SlaveObject obj = promise.value<SlaveObject>();
        if (obj.uid() <= 0) {
            obj.set_uid(200);
        }
        // Ensure link uid is always set from current master.
        if (masterValid()) {
            const MasterObject masterObj = masterVariantObject().value<MasterObject>();
            obj.set_link_uid(masterObj.uid());
        }
        return QVariant::fromValue(obj);
    }

    QVariant workerEditObject(const QVariant & promise) override { return promise; }
    QVariant workerDeleteObject(const QVariant & promise) override { return promise; }

    bool masterValid() override
    {
        const QVariant varObject = masterVariantObject();
        if (!varObject.isValid()) {
            return false;
        }
        const MasterObject obj = varObject.value<MasterObject>();
        return obj.uid() > 0;
    }

private:
    std::vector<SlaveObject> m_slaveData;
};

class GrpcMasterSlaveIntegrationFixture : public ::testing::Test
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

        masterModel = std::make_unique<MasterModel>(mainWindow.get());
        slaveModel = std::make_unique<SlaveModel>(mainWindow.get());

        masterProxy = std::make_unique<GrpcProxySortFilterModel>(masterModel.get(), QList<int>{}, mainWindow.get());
        slaveProxy = std::make_unique<GrpcProxySortFilterModel>(slaveModel.get(), QList<int>{}, mainWindow.get());

        masterView = std::make_unique<GrpcTableView>(mainWindow.get());
        slaveView = std::make_unique<GrpcTableView>(mainWindow.get());

        masterForm = std::make_unique<MasterForm>(mainWindow.get());
        slaveForm = std::make_unique<SlaveForm>(mainWindow.get());

        masterController = std::make_unique<MasterTemplateController>(masterProxy.get(), masterView.get(), masterForm.get(), mainWindow.get());
        slaveController = std::make_unique<SlaveTemplateController>(slaveProxy.get(), slaveView.get(), slaveForm.get(), mainWindow.get());

        // Prevent modal warning dialogs during tests.
        QObject::disconnect(masterController.get(), &GrpcTemplateController::warning, masterView.get(), &GrpcTableView::showWarning);
        QObject::disconnect(slaveController.get(), &GrpcTemplateController::warning, slaveView.get(), &GrpcTableView::showWarning);

        // Ensure toolbars exist (some controller code expects addActionBars to be callable).
        auto * masterTb = new QToolBar(mainWindow.get());
        auto * slaveTb = new QToolBar(mainWindow.get());
        mainWindow->addToolBar(masterTb);
        mainWindow->addToolBar(slaveTb);

        masterController->addActionBars(mainWindow.get(), mainWindow->menuBar(), masterTb, mainWindow->statusBar());
        slaveController->addActionBars(mainWindow.get(), mainWindow->menuBar(), slaveTb, mainWindow->statusBar());

        msController = std::make_unique<GrpcMasterSlaveController>(masterController.get(), slaveController.get(), mainWindow.get());

        // Seed data.
        std::vector<MasterObject> masters;
        MasterObject m1;
        m1.set_uid(1);
        m1.set_name("M1");
        masters.push_back(m1);
        MasterObject m2;
        m2.set_uid(2);
        m2.set_name("M2");
        masters.push_back(m2);
        masterController->setData(std::move(masters));

        std::vector<SlaveObject> slaves;
        SlaveObject s1;
        s1.set_uid(10);
        s1.set_link_uid(1);
        s1.set_phone("111");
        slaves.push_back(s1);
        SlaveObject s2;
        s2.set_uid(11);
        s2.set_link_uid(1);
        s2.set_phone("112");
        slaves.push_back(s2);
        SlaveObject s3;
        s3.set_uid(20);
        s3.set_link_uid(2);
        s3.set_phone("221");
        slaves.push_back(s3);
        slaveController->setData(std::move(slaves));

        // Load master list.
        ASSERT_TRUE(QMetaObject::invokeMethod(masterController.get(), "refresh_all"));

        processEventsUntil([this]() {
            return masterProxy->rowCount() == 2;
        });

        QCoreApplication::processEvents();
    }

    static QApplication * s_app;

    std::unique_ptr<QMainWindow> mainWindow;

    std::unique_ptr<MasterModel> masterModel;
    std::unique_ptr<SlaveModel> slaveModel;

    std::unique_ptr<GrpcProxySortFilterModel> masterProxy;
    std::unique_ptr<GrpcProxySortFilterModel> slaveProxy;

    std::unique_ptr<GrpcTableView> masterView;
    std::unique_ptr<GrpcTableView> slaveView;

    std::unique_ptr<MasterForm> masterForm;
    std::unique_ptr<SlaveForm> slaveForm;

    std::unique_ptr<MasterTemplateController> masterController;
    std::unique_ptr<SlaveTemplateController> slaveController;

    std::unique_ptr<GrpcMasterSlaveController> msController;
};

QApplication * GrpcMasterSlaveIntegrationFixture::s_app = nullptr;

} // namespace

TEST_F(GrpcMasterSlaveIntegrationFixture, SelectingMasterFiltersSlaveRows)
{
    // Select master uid=1 => should show 2 slave rows.
    masterView->setModel(masterProxy.get());
    slaveView->setModel(slaveProxy.get());

    masterView->select(0);

    processEventsUntil([this]() {
        return slaveProxy->rowCount() == 2;
    });

    EXPECT_EQ(slaveProxy->rowCount(), 2);

    // Select master uid=2 => should show 1 slave row.
    masterView->select(1);

    processEventsUntil([this]() {
        return slaveProxy->rowCount() == 1;
    });

    EXPECT_EQ(slaveProxy->rowCount(), 1);

    const QVariant v = slaveModel->variantObject(0);
    ASSERT_TRUE(v.isValid());
    const SlaveObject obj = v.value<SlaveObject>();
    EXPECT_EQ(obj.link_uid(), 2);
    EXPECT_EQ(obj.phone(), "221");
}

TEST_F(GrpcMasterSlaveIntegrationFixture, SlaveInsertUsesCurrentMasterUid)
{
    masterView->setModel(masterProxy.get());
    slaveView->setModel(slaveProxy.get());

    // Select master uid=2.
    masterView->select(1);
    processEventsUntil([this]() {
        return slaveProxy->rowCount() == 1;
    });

    // Start insert in slave and save.
    ASSERT_TRUE(QMetaObject::invokeMethod(slaveController.get(), "add_new_record"));
    QCoreApplication::processEvents();

    slaveForm->phoneEdit()->setText("222");

    ASSERT_TRUE(QMetaObject::invokeMethod(slaveController.get(), "save_record"));

    processEventsUntil([this]() {
        return slaveModel->rowCount() == 2;
    });

    ASSERT_EQ(slaveModel->rowCount(), 2);

    // New row should have link_uid=2.
    const QVariant vNew = slaveModel->variantObject(1);
    ASSERT_TRUE(vNew.isValid());

    const SlaveObject obj = vNew.value<SlaveObject>();
    EXPECT_EQ(obj.link_uid(), 2);
    EXPECT_EQ(obj.phone(), "222");
}

TEST_F(GrpcMasterSlaveIntegrationFixture, SlaveEditKeepsLinkUidUnchanged)
{
    masterView->setModel(masterProxy.get());
    slaveView->setModel(slaveProxy.get());

    // Select master uid=1 so we see two slave rows.
    masterView->select(0);
    processEventsUntil([this]() {
        return slaveProxy->rowCount() == 2;
    });

    // Select first slave row and edit its phone.
    slaveView->select(0);
    QCoreApplication::processEvents();

    ASSERT_TRUE(QMetaObject::invokeMethod(slaveController.get(), "edit_record"));
    QCoreApplication::processEvents();

    slaveForm->phoneEdit()->setText("999");
    ASSERT_TRUE(QMetaObject::invokeMethod(slaveController.get(), "save_record"));

    // The slave controller loads a filtered dataset into the slave model (not the full 3-row seed).
    processEventsUntil([this]() {
        if (slaveModel->rowCount() != 2) {
            return false;
        }
        const QVariant vEdited = slaveModel->variantObject(0);
        if (!vEdited.isValid()) {
            return false;
        }
        const SlaveObject obj = vEdited.value<SlaveObject>();
        return obj.phone() == "999";
    });

    ASSERT_EQ(slaveModel->rowCount(), 2);

    // Ensure the edited row kept the original link_uid=1.
    const QVariant vEdited = slaveModel->variantObject(0);
    ASSERT_TRUE(vEdited.isValid());
    const SlaveObject obj = vEdited.value<SlaveObject>();
    EXPECT_EQ(obj.link_uid(), 1);
    EXPECT_EQ(obj.phone(), "999");
}

TEST_F(GrpcMasterSlaveIntegrationFixture, SwitchingMasterDuringSlaveInsertIsSafeAndNextInsertUsesNewMaster)
{
    masterView->setModel(masterProxy.get());
    slaveView->setModel(slaveProxy.get());

    // Start on master uid=1.
    masterView->select(0);
    processEventsUntil([this]() {
        return slaveProxy->rowCount() == 2;
    });

    // Begin inserting a slave record under master=1.
    ASSERT_TRUE(QMetaObject::invokeMethod(slaveController.get(), "add_new_record"));
    QCoreApplication::processEvents();

    slaveForm->phoneEdit()->setText("TEMP");

    // Switch master to uid=2 while insert is in progress.
    masterView->select(1);
    processEventsUntil([this]() {
        return slaveProxy->rowCount() == 1;
    });

    // Escape to avoid relying on any specific UX around master-switch cancellation.
    ASSERT_TRUE(QMetaObject::invokeMethod(slaveController.get(), "escape"));
    QCoreApplication::processEvents();

    // Now insert again; new slave object must link to master uid=2.
    ASSERT_TRUE(QMetaObject::invokeMethod(slaveController.get(), "add_new_record"));
    QCoreApplication::processEvents();

    slaveForm->phoneEdit()->setText("223");
    ASSERT_TRUE(QMetaObject::invokeMethod(slaveController.get(), "save_record"));

    // Under master uid=2, the slave model contains only the filtered rows.
    processEventsUntil([this]() {
        return slaveModel->rowCount() == 2;
    });

    bool found = false;
    for (int row = 0; row < slaveModel->rowCount(); ++row) {
        const QVariant v = slaveModel->variantObject(row);
        if (!v.isValid()) {
            continue;
        }
        const SlaveObject obj = v.value<SlaveObject>();
        if (obj.phone() == "223") {
            found = true;
            EXPECT_EQ(obj.link_uid(), 2);
        }
    }

    EXPECT_TRUE(found);
}
