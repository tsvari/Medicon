#include <gtest/gtest.h>

#include "GrpcTemplateController.h"
#include "GrpcProxySortFilterModel.h"
#include "GrpcTableView.h"

#include "GrpcDataContainer.hpp"
#include "GrpcForm.h"
#include "GrpcObjectTableModel.h"
#include "GrpcObjectWrapper.hpp"
#include "TestSharedUtility.h"

#include "GrpcViewNavigator.h"

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

#include <functional>
#include <memory>

namespace {

QAction * findAction(QObject * parent, const char * objectName)
{
    if (!parent) {
        return nullptr;
    }
    return parent->findChild<QAction *>(objectName);
}

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
        container->addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    }
};

class TemplateControllerTestForm : public GrpcForm
{
public:
    explicit TemplateControllerTestForm(QWidget * parent = nullptr)
        : GrpcForm(new GrpcObjectWrapper<MasterObject>(), nullptr, parent)
    {
        initializeForm();
    }

    void initializeForm() override
    {
        auto * wrapper = dynamic_cast<GrpcObjectWrapper<MasterObject> *>(objectWrapper());
        if (!wrapper) {
            return;
        }

        m_nameEdit = new QLineEdit(this);
        m_nameEdit->setObjectName("Name");

        m_heightEdit = new QLineEdit(this);
        m_heightEdit->setObjectName("Height");

        wrapper->addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
        wrapper->addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    }

    QVariant defaultObject() override
    {
        MasterObject obj;
        obj.set_uid(0);
        obj.set_name("");
        obj.set_height(0);
        return QVariant::fromValue(obj);
    }

    void simulateUserEdit()
    {
        emit formContentChanaged();
    }

public:
    QLineEdit * m_nameEdit = nullptr;
    QLineEdit * m_heightEdit = nullptr;
};

class TestGrpcTemplateController : public GrpcTemplateController
{
public:
    explicit TestGrpcTemplateController(GrpcProxySortFilterModel * proxyModel,
                                        GrpcTableView * tableView,
                                        GrpcForm * form,
                                        QObject * parent = nullptr)
        : GrpcTemplateController(proxyModel, tableView, form, nullptr, parent)
    {
    }

    int workerModelDataCalls = 0;
    QStringList validityErrors;

    int currentPageForTest()
    {
        return currentNavigatorPage();
    }

protected:
    void workerModelData() override
    {
        ++workerModelDataCalls;

        // Keep any UI-bound signals on the controller thread.
        auto container = std::make_shared<GrpcDataContainer<MasterObject>>(std::vector<MasterObject>{});
        QMetaObject::invokeMethod(this, [this, container]() {
            emit populateModel(container);
        }, Qt::QueuedConnection);
    }

    QVariant workerAddNewObject(const QVariant & promise) override
    {
        MasterObject obj = promise.value<MasterObject>();
        if (obj.uid() <= 0) {
            obj.set_uid(123);
        }
        return QVariant::fromValue(obj);
    }

    QVariant workerEditObject(const QVariant & promise) override
    {
        return promise;
    }

    QVariant workerDeleteObject(const QVariant & promise) override
    {
        return promise;
    }

    QStringList checkObjectValidity() override
    {
        return validityErrors;
    }

    void startLoadingData() override
    {
        // Track that this path was invoked, but keep the base behavior.
        GrpcTemplateController::startLoadingData();
    }
};

class GrpcTemplateControllerFixture : public ::testing::Test
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

        auto * model = new TestGrpcObjectTableModel(mainWindow.get());
        sourceModel = model;

        proxyModel = std::make_unique<GrpcProxySortFilterModel>(model, QList<int>{}, mainWindow.get());
        view = std::make_unique<GrpcTableView>(mainWindow.get());
        form = std::make_unique<TemplateControllerTestForm>(mainWindow.get());

        controller = std::make_unique<TestGrpcTemplateController>(proxyModel.get(), view.get(), form.get(), mainWindow.get());

        // In production the controller forwards warnings to the view which shows a modal QMessageBox.
        // For unit tests, disconnect this to avoid blocking dialogs.
        QObject::disconnect(controller.get(), &GrpcTemplateController::warning, view.get(), &GrpcTableView::showWarning);

        // Ensure menu/toolbar exist so focus wiring can't crash.
        auto * baseToolBar = new QToolBar(mainWindow.get());
        mainWindow->addToolBar(baseToolBar);

        controller->addActionBars(mainWindow.get(), mainWindow->menuBar(), baseToolBar, mainWindow->statusBar());

        QCoreApplication::processEvents();
    }

    void TearDown() override
    {
        controller.reset();
        form.reset();
        view.reset();
        proxyModel.reset();
        mainWindow.reset();

        // sourceModel is parented to mainWindow.
        sourceModel = nullptr;
    }

    static QApplication * s_app;

    std::unique_ptr<QMainWindow> mainWindow;
    TestGrpcObjectTableModel * sourceModel = nullptr;
    std::unique_ptr<GrpcProxySortFilterModel> proxyModel;
    std::unique_ptr<GrpcTableView> view;
    std::unique_ptr<TemplateControllerTestForm> form;
    std::unique_ptr<TestGrpcTemplateController> controller;
};

QApplication * GrpcTemplateControllerFixture::s_app = nullptr;

} // namespace

TEST_F(GrpcTemplateControllerFixture, ConstructorCreatesStandardActions)
{
    auto * refresh = findAction(controller.get(), "actionRefresh");
    auto * addNew = findAction(controller.get(), "actionAddNew");
    auto * edit = findAction(controller.get(), "actionEdit");
    auto * del = findAction(controller.get(), "actionDelete");
    auto * save = findAction(controller.get(), "actionSave");

    ASSERT_NE(refresh, nullptr);
    ASSERT_NE(addNew, nullptr);
    ASSERT_NE(edit, nullptr);
    ASSERT_NE(del, nullptr);
    ASSERT_NE(save, nullptr);

    EXPECT_TRUE(refresh->isEnabled());
    EXPECT_TRUE(addNew->isEnabled());
    EXPECT_FALSE(edit->isEnabled());
    EXPECT_FALSE(del->isEnabled());
    EXPECT_FALSE(save->isEnabled());
}

TEST_F(GrpcTemplateControllerFixture, ClearModelResetsNavigator)
{
    GrpcViewNavigator navigator;
    controller->addNavigator(&navigator);

    navigator.addPages(3);
    navigator.selectPage(2);
    ASSERT_EQ(navigator.currentPage(), 2);

    controller->clearModel();
    QCoreApplication::processEvents();

    EXPECT_EQ(navigator.currentPage(), -1);
    EXPECT_EQ(controller->currentPageForTest(), 1);
}

TEST_F(GrpcTemplateControllerFixture, AddNewTransitionsToInsertState)
{
    auto * addNew = findAction(controller.get(), "actionAddNew");
    auto * save = findAction(controller.get(), "actionSave");
    auto * edit = findAction(controller.get(), "actionEdit");
    auto * del = findAction(controller.get(), "actionDelete");

    ASSERT_TRUE(QMetaObject::invokeMethod(controller.get(), "add_new_record"));
    QCoreApplication::processEvents();

    EXPECT_FALSE(addNew->isEnabled());
    EXPECT_TRUE(save->isEnabled());
    EXPECT_FALSE(edit->isEnabled());
    EXPECT_FALSE(del->isEnabled());
}

TEST_F(GrpcTemplateControllerFixture, EscapeFromInsertEmitsReadonlyAndFinishesSave)
{
    ASSERT_TRUE(QMetaObject::invokeMethod(controller.get(), "add_new_record"));
    QCoreApplication::processEvents();

    QSignalSpy readonlySpy(controller.get(), SIGNAL(makeFormReadonly(bool)));
    QSignalSpy finishSpy(controller.get(), SIGNAL(finishSave()));

    ASSERT_TRUE(QMetaObject::invokeMethod(controller.get(), "escape"));
    QCoreApplication::processEvents();

    ASSERT_GE(finishSpy.count(), 1);
    ASSERT_GE(readonlySpy.count(), 1);
    EXPECT_TRUE(readonlySpy.takeFirst().at(0).toBool());
}

TEST_F(GrpcTemplateControllerFixture, SelectingRowMovesToBrowsingState)
{
    MasterObject obj;
    obj.set_uid(1);
    obj.set_name("Alice");
    obj.set_height(170);

    sourceModel->addNewObject(QVariant::fromValue(obj));

    processEventsUntil([this]() {
        return proxyModel->rowCount() == 1 && view->currentIndex().isValid();
    });

    auto * edit = findAction(controller.get(), "actionEdit");
    auto * del = findAction(controller.get(), "actionDelete");
    auto * save = findAction(controller.get(), "actionSave");

    ASSERT_NE(edit, nullptr);
    ASSERT_NE(del, nullptr);
    ASSERT_NE(save, nullptr);

    EXPECT_TRUE(edit->isEnabled());
    EXPECT_TRUE(del->isEnabled());
    EXPECT_FALSE(save->isEnabled());
}

TEST_F(GrpcTemplateControllerFixture, FormContentChangedMovesToEditState)
{
    MasterObject obj;
    obj.set_uid(1);
    obj.set_name("Alice");
    obj.set_height(170);

    sourceModel->addNewObject(QVariant::fromValue(obj));
    processEventsUntil([this]() {
        return view->currentIndex().isValid();
    });

    form->simulateUserEdit();
    QCoreApplication::processEvents();

    auto * addNew = findAction(controller.get(), "actionAddNew");
    auto * edit = findAction(controller.get(), "actionEdit");
    auto * del = findAction(controller.get(), "actionDelete");
    auto * save = findAction(controller.get(), "actionSave");

    EXPECT_FALSE(addNew->isEnabled());
    EXPECT_FALSE(edit->isEnabled());
    EXPECT_FALSE(del->isEnabled());
    EXPECT_TRUE(save->isEnabled());
}

TEST_F(GrpcTemplateControllerFixture, SaveInsertAddsRowToModel)
{
    controller->validityErrors.clear();

    ASSERT_TRUE(QMetaObject::invokeMethod(controller.get(), "add_new_record"));
    QCoreApplication::processEvents();

    form->m_nameEdit->setText("Bob");
    form->m_heightEdit->setText("180");

    ASSERT_TRUE(QMetaObject::invokeMethod(controller.get(), "save_record"));

    processEventsUntil([this]() {
        return sourceModel->rowCount() == 1;
    });

    EXPECT_EQ(sourceModel->rowCount(), 1);

    // After save the template should return to browsing.
    auto * save = findAction(controller.get(), "actionSave");
    auto * edit = findAction(controller.get(), "actionEdit");

    processEventsUntil([save]() {
        return save && !save->isEnabled();
    });

    EXPECT_FALSE(save->isEnabled());
    EXPECT_TRUE(edit->isEnabled());
}

TEST_F(GrpcTemplateControllerFixture, SaveWithValidationErrorsEmitsWarning)
{
    controller->validityErrors = { "Name is required" };

    ASSERT_TRUE(QMetaObject::invokeMethod(controller.get(), "add_new_record"));
    QCoreApplication::processEvents();

    QSignalSpy warningSpy(controller.get(), SIGNAL(warning(QString,QString)));

    ASSERT_TRUE(QMetaObject::invokeMethod(controller.get(), "save_record"));
    QCoreApplication::processEvents();

    ASSERT_GE(warningSpy.count(), 1);
    EXPECT_EQ(sourceModel->rowCount(), 0);
}

TEST_F(GrpcTemplateControllerFixture, DeleteRemovesRow)
{
    MasterObject obj;
    obj.set_uid(1);
    obj.set_name("Alice");
    obj.set_height(170);

    sourceModel->addNewObject(QVariant::fromValue(obj));

    processEventsUntil([this]() {
        return view->currentIndex().isValid();
    });

    ASSERT_TRUE(QMetaObject::invokeMethod(controller.get(), "delete_record"));

    processEventsUntil([this]() {
        return sourceModel->rowCount() == 0;
    });

    EXPECT_EQ(sourceModel->rowCount(), 0);

    auto * edit = findAction(controller.get(), "actionEdit");
    auto * del = findAction(controller.get(), "actionDelete");
    auto * save = findAction(controller.get(), "actionSave");

    EXPECT_FALSE(edit->isEnabled());
    EXPECT_FALSE(del->isEnabled());
    EXPECT_FALSE(save->isEnabled());
}
