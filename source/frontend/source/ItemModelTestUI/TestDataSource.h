#ifndef TESTDATASOURCE_H
#define TESTDATASOURCE_H

#include <QItemSelection>
#include <QAbstractItemModel>
#include <QVariant>
#include <QLineEdit>
#include <QTableView>
#include <QToolBar>
#include <QMenuBar>
#include <QMenu>
#include <QThread>
#include <QRandomGenerator>

#include "GrpcObjectTableModel.h"
#include "GrpcDataContainer.hpp"
#include "TypeToStringFormatter.h"
#include "GrpcForm.h"
#include "GrpcProxySortFilterModel.h"
#include "GrpcTemplateController.h"
#include "GrpcThreadWorker.h"
#include "TestSharedUtility.h"
#include "include_frontend_util.h"

using CommonUtil::sqlRowOffset;



class GprcTestLevelObject
{
public:
    ~GprcTestLevelObject(){}
    GprcTestLevelObject(){}
    GprcTestLevelObject(int32_t uid, const std::string & name)
        : m_uid(uid)
        , m_name(name)
    {
    }

    int32_t uid() const {return m_uid;}
    void set_uid(int32_t value) {m_uid = value;}

    const std::string & name() const {return m_name;}
    void set_name(const std::string & value) {m_name = value;}

private:
    int32_t m_uid;
    std::string m_name;
};

namespace TestModelData {
static std::vector<MasterObject> masterData(int & nameInt) {
    std::vector<MasterObject> objects;
    auto currentDateTime = [=]() {
        return TimeFormatHelper::chronoNow().count();
    };
    static int incr = 0;
    auto increament = [currentDateTime]() {
        incr += 11111;
        return (currentDateTime() + incr);
    };

    MasterObject obj1;
    obj1.set_uid(1);
    //obj1.set_name("Givi");
    obj1.set_name(std::to_string(++nameInt));
    obj1.set_date(increament());
    obj1.set_time(increament());
    obj1.set_date_time(increament());
    obj1.set_date_time_no_sec(increament());
    obj1.set_height(168);
    obj1.set_salary(12.15);
    obj1.set_married(false);
    obj1.set_married_name("No");
    obj1.set_level(2);
    obj1.set_level_name("Level2");
    objects.push_back(obj1);

    MasterObject obj2;
    obj2.set_uid(2);
    //obj2.set_name("Keto");
    obj2.set_name(std::to_string(++nameInt));
    obj2.set_date(increament());
    obj2.set_time(increament());
    obj2.set_date_time(increament());
    obj2.set_date_time_no_sec(increament());
    obj2.set_height(164);
    obj2.set_salary(30.557);
    obj2.set_married(true);
    obj2.set_married_name("Yes");
    obj2.set_level(1);
    obj2.set_level_name("Level1");
    objects.push_back(obj2);

    MasterObject obj3;
    obj3.set_uid(3);
    //obj3.set_name("Vakho");
    obj3.set_name(std::to_string(++nameInt));
    obj3.set_date(increament());
    obj3.set_time(increament());
    obj3.set_date_time(increament());
    obj3.set_date_time_no_sec(increament());
    obj3.set_height(175);
    obj3.set_salary(135000.567);
    obj3.set_married(true);
    obj3.set_married_name("Yes");
    obj3.set_level(3);
    obj3.set_level_name("Level3");
    objects.push_back(obj3);

    MasterObject obj4;
    obj4.set_uid(4);
    //obj4.set_name("Elene");
    obj4.set_name(std::to_string(++nameInt));
    obj4.set_date(increament());
    obj4.set_time(increament());
    obj4.set_date_time(increament());
    obj4.set_date_time_no_sec(increament());
    obj4.set_height(155);
    obj4.set_salary(567);
    obj4.set_married(false);
    obj4.set_married_name("No");
    obj4.set_level(5);
    obj4.set_level_name("Level5");
    objects.push_back(obj4);

    MasterObject obj5;
    obj5.set_uid(5);
    //obj5.set_name("Teona");
    obj5.set_name(std::to_string(++nameInt));
    obj5.set_date(increament());
    obj5.set_time(increament());
    obj5.set_date_time(increament());
    obj5.set_date_time_no_sec(increament());
    obj5.set_height(166);
    obj5.set_salary(5.12);
    obj5.set_married(true);
    obj5.set_married_name("Yes");
    obj5.set_level(4);
    obj5.set_level_name("Level4");
    objects.push_back(obj5);

    MasterObject obj6;
    obj6.set_uid(6);
    //obj6.set_name("Tsio");
    obj6.set_name(std::to_string(++nameInt));
    obj6.set_date(increament());
    obj6.set_time(increament());
    obj6.set_date_time(increament());
    obj6.set_date_time_no_sec(increament());
    obj6.set_height(166);
    obj6.set_salary(5.123);
    obj6.set_married(true);
    obj6.set_married_name("Yes");
    obj6.set_level(4);
    obj6.set_level_name("Level4");
    objects.push_back(obj6);

    return objects;
}

static std::vector<SlaveObject> slaveData()
{
    std::vector<SlaveObject> objects {
        {1, 1, "123 334 5678"},
        {2, 1, "445 575 8778"},
        {3, 1, "453 464 3464"},
        {4, 1, "343 235 4364"},
        {5, 2, "454 435 3466"},
        {6, 2, "234 236 6588"},
        {7, 3, "677 683 4378"},
        {8, 3, "325 325 7679"},
        {9, 3, "368 568 2365"},
        {10, 4, "235 436 7988"},
        {11, 4, "546 325 4345"},
        {12, 5, "435 577 9870"},
        {13, 5, "544 870 8708"},
        {14, 5, "323 679 5670"},
        {15, 5, "234 346 4799"},
        {16, 5, "356 578 5758"}
    };

    return objects;
}

static std::vector<GprcTestLevelObject> comboLevelData()
{
    std::vector<GprcTestLevelObject> objects {
        {1, "Level1"},
        {2, "Level2"},
        {3, "Level3"},
        {4, "Level4"},
        {5, "Level5"}
    };

    return objects;
}
}

class GrpcTestObjectTableModel : public GrpcObjectTableModel
{
    Q_OBJECT

public:
    explicit GrpcTestObjectTableModel(std::vector<MasterObject> && data, QObject *parent = nullptr) :
        GrpcObjectTableModel(new GrpcDataContainer<MasterObject>(std::move(data)), parent)
    {
        initializeModel();
        initializeContainer();
    }

    explicit GrpcTestObjectTableModel(QObject *parent = nullptr) :
        GrpcObjectTableModel( new GrpcDataContainer<MasterObject>(), parent)
    {
    }

    void initializeModel() override {
        GrpcDataContainer<MasterObject> * container = dynamic_cast<GrpcDataContainer<MasterObject>*>(objectContainer());

        container->addProperty("Uid", DataInfo::String, &MasterObject::set_uid, &MasterObject::uid);
        container->addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
        container->addProperty("Date", DataInfo::Date, &MasterObject::set_date, &MasterObject::date);
        container->addProperty("Time", DataInfo::Time, &MasterObject::set_time, &MasterObject::time);
        container->addProperty("DateTime", DataInfo::DateTime, &MasterObject::set_date_time, &MasterObject::date_time);
        container->addProperty("DateTimeNosec", DataInfo::DateTimeNoSec, &MasterObject::set_date_time_no_sec, &MasterObject::date_time_no_sec);
        container->addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
        container->addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
        container->addProperty("Married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
        container->addProperty("Married Name", DataInfo::String, &MasterObject::set_married_name, &MasterObject::married_name);
        container->addProperty("Level", DataInfo::Int, &MasterObject::set_level, &MasterObject::level);
        container->addProperty("Level Name", DataInfo::String, &MasterObject::set_level_name, &MasterObject::level_name);
    }
};

class GrpcTestSlaveObjectTableModel : public GrpcObjectTableModel
{
    Q_OBJECT

public:
    explicit GrpcTestSlaveObjectTableModel(std::vector<SlaveObject> && data, QObject *parent = nullptr) :
        GrpcObjectTableModel(new GrpcDataContainer<SlaveObject>(std::move(data)), parent)
    {
        initializeModel();
        initializeContainer();
    }

    explicit GrpcTestSlaveObjectTableModel(QObject *parent = nullptr) :
        GrpcObjectTableModel(new GrpcDataContainer<SlaveObject>(), parent)
    {
    }

    void initializeModel() override {
        GrpcDataContainer<SlaveObject> * container = dynamic_cast<GrpcDataContainer<SlaveObject>*>(objectContainer());

        container->addProperty("Uid", DataInfo::Int, &SlaveObject::set_uid, &SlaveObject::uid);
        container->addProperty("LinkUid", DataInfo::Int, &SlaveObject::set_link_uid, &SlaveObject::link_uid);
        container->addProperty("Phone", DataInfo::String, &SlaveObject::set_phone, &SlaveObject::phone);
    }
};

class GrpcTestLevelObjectTableModel : public GrpcObjectTableModel
{
    Q_OBJECT

public:
    explicit GrpcTestLevelObjectTableModel(std::vector<GprcTestLevelObject> && data, QObject *parent = nullptr) :
        GrpcObjectTableModel(new GrpcDataContainer<GprcTestLevelObject>(std::move(data)), parent)
    {
        initializeModel();
        initializeContainer();
    }

    void initializeModel() override {
        GrpcDataContainer<GprcTestLevelObject> * container = dynamic_cast<GrpcDataContainer<GprcTestLevelObject>*>(objectContainer());

        container->addProperty("Uid", DataInfo::Int, &GprcTestLevelObject::set_uid, &GprcTestLevelObject::uid);
        container->addProperty("Level", DataInfo::String, &GprcTestLevelObject::set_name, &GprcTestLevelObject::name);
    }
};

class MasterForm : public GrpcForm
{
    Q_OBJECT

public:
    explicit MasterForm(QWidget *parent = nullptr)
        : GrpcForm( new GrpcObjectWrapper<MasterObject>(), nullptr, parent){
    }

    void initializeForm() override {
        GrpcObjectWrapper<MasterObject> * wrapper = dynamic_cast<GrpcObjectWrapper<MasterObject>*>(objectWrapper());

        //wrapper()->addProperty("Uid", DataInfo::String, &MasterObject::set_uid, &MasterObject::uid);
        wrapper->addProperty("nameEdit", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
        wrapper->addProperty("dateEdit", DataInfo::Date, &MasterObject::set_date, &MasterObject::date);
        wrapper->addProperty("timeEdit", DataInfo::Time, &MasterObject::set_time, &MasterObject::time);
        wrapper->addProperty("dateTimeEdit", DataInfo::DateTime, &MasterObject::set_date_time, &MasterObject::date_time);
        wrapper->addProperty("dateTimeEditNoSec", DataInfo::DateTimeNoSec, &MasterObject::set_date_time_no_sec, &MasterObject::date_time_no_sec);
        wrapper->addProperty("heightEdit", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
        wrapper->addProperty("salaryEdit", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
        wrapper->addProperty("marriedCheckBox", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
        wrapper->addProperty("marriedCheckBox", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
        wrapper->addProperty("levelCombo", DataInfo::Int, &MasterObject::set_level, &MasterObject::level);
        wrapper->addProperty("levelCombo", DataInfo::String, &MasterObject::set_level_name, &MasterObject::level_name, DataMask::ComboEditMask);
        wrapper->addProperty("marriedCheckBox", DataInfo::String, &MasterObject::set_married_name, &MasterObject::married_name, DataMask::CheckBoxMask, "Yes", "No");
    }

    QVariant defaultObject() override {
        MasterObject object;
        return QVariant::fromValue<MasterObject>(object);
    }
};

class SlaveForm : public GrpcForm
{
    Q_OBJECT

public:
    explicit SlaveForm(QWidget *parent = nullptr) :
        GrpcForm(new GrpcObjectWrapper<SlaveObject>(), new GrpcObjectWrapper<MasterObject>(), parent){

    }

    void initializeForm() override {
        GrpcObjectWrapper<SlaveObject> * wrapper = dynamic_cast<GrpcObjectWrapper<SlaveObject>*>(objectWrapper());
        //wrapper->addProperty("Uid", DataInfo::Int, &SlaveObject::set_uid, &SlaveObject::uid);
        //wrapper->addProperty("LinkUid", DataInfo::Int, &SlaveObject::set_link_uid, &SlaveObject::link_uid);
        wrapper->addProperty("phoneEdit", DataInfo::String, &SlaveObject::set_phone, &SlaveObject::phone);
    }

    QVariant defaultObject() override {
        QVariant varObject = masterVariantObject();
        Q_ASSERT(varObject.isValid());

        MasterObject masterObject = varObject.value<MasterObject>();

        SlaveObject formObject;
        formObject.set_link_uid(masterObject.uid());
        return QVariant::fromValue<SlaveObject>(formObject);
    }
};

class MasterTemplate : public GrpcTemplateController
{
    Q_OBJECT

public: explicit MasterTemplate(GrpcProxySortFilterModel * model,
                            GrpcTableView * tableView, GrpcForm * form, QObject *parent = nullptr) :
        GrpcTemplateController(model,
                               tableView,
                               form,
                               nullptr,// It's master only
                               parent)
    {
    }

    void addActionBars(QMainWindow * mainWindow, QMenuBar * menuBar, QToolBar * toolBar, QStatusBar * statusBar) override {
        // Generate standard tempalte actions and in parent class
        GrpcTemplateController::addActionBars(mainWindow, menuBar, toolBar, statusBar);

        // Create new actions and connect to slots
        m_testAction = new QAction("Test", this);
        m_testAction->setIcon(QIcon(":/icons/exit.png"));
        m_testAction->setStatusTip(tr("Testing tool button"));
        connect(m_testAction, &QAction::triggered, this, &MasterTemplate::testSlot);

        // Add new actions to menu and/or toolbar
        if(QMenu * menu = templateMenu()) {
            menu->addSeparator();
            menu->addAction(m_testAction);
        }

        if( QToolBar * toobar = templateToolBar()) {
            toobar->addSeparator();
            toobar->addAction(m_testAction);
        }

        // Change context menu if any
        if(QMenu * menu = contextMenu()) {
            menu->addSeparator();
            menu->addAction(m_testAction);
        }
    }

    void updateState() override {
        // Calculate state for generic bar and menu
        GrpcTemplateController::updateState();

        // More enable/disable custom actions

    }

    void workerModelData() override {
        JsonParameterFormatter criterias = searchCriterias();
        // Dont need master object in thinscase

        // Simulating waiting for a response from the server
        QThread::msleep(5000);

        std::vector<MasterObject> dataSource;
        int nameInt = 0;
        for(int i = 0; i < 100; i++) {
            std::vector<MasterObject>  dataSource2 = TestModelData::masterData(nameInt);
            std::copy(dataSource2.begin(), dataSource2.end(), std::back_inserter(dataSource));
        }

        // run count request to server
        int recordCount = dataSource.size();

        // Calculate offset (where from start)
        int ofset = sqlRowOffset(currentNavigatorPage(), maxPages(), recordCount);

        // Init navigator
        emit navigatorRecordCount(recordCount);

        // Send data request to server and get data source
        std::vector<MasterObject> filteredDataSource(dataSource.begin() + ofset, dataSource.begin() + std::min(ofset + maxPages(), recordCount));

        // Populate model
        emit populateModel(
            std::make_shared<GrpcDataContainer<MasterObject>>(
                std::move(filteredDataSource)
                )
            );
    }

    QStringList checkObjectValidity() override {
        QStringList errors;
        QVariant masterFormObject = formObject();
        if(!masterFormObject.isValid()) {
            errors << tr("Something is wrong with the form data!");
            return errors;
        }
        MasterObject nativeObject = masterFormObject.value<MasterObject>();

        if(nativeObject.name().empty()) {
            errors << tr("The 'Name' field must not be empty!");
        }
        if(nativeObject.height() <= 0) {
            errors << tr("The 'Height' field must be greater than 0!");
        }
        if(nativeObject.level() > 5 || nativeObject.level() <=0 || nativeObject.level_name().empty()) {
            errors << tr("Select a 'Level' from the combo list!");
        }
        if(nativeObject.salary() <= 0) {
            errors << tr("Enter the correct 'Salary' amount!");
        }
        return errors;
    }

    QVariant workerAddNewObject(const QVariant & promise) override {
        // Simulating waiting for a response from the server
        QThread::msleep(300);

        QVariant variantObject = formObject();
        MasterObject masterFormObject = variantObject.value<MasterObject>();

        // We send the object to the server and, if there are no exceptions, return it to the game
        masterFormObject.set_uid(randomInt(1000, 100000));

        return QVariant::fromValue<MasterObject>(masterFormObject);
    }
    QVariant workerEditObject(const QVariant & promise) override {
        // Simulating waiting for a response from the server
        QThread::msleep(300);

        // We send the object to the server and, if there are no exceptions, return it to the game
        return promise;
    }
    QVariant workerDeleteObject(const QVariant & promise) override {
        // Simulating waiting for a response from the server
        QThread::msleep(300);

        // We send the object to the server and, if there are no exceptions, return it to the game
        return promise;
    }

private slots:
    void testSlot() {

    }
private:
    QAction * m_testAction;
};


class SlaveTemplate : public GrpcTemplateController
{
    Q_OBJECT

public: explicit SlaveTemplate(GrpcProxySortFilterModel * proxyModel, GrpcTableView * tableView, GrpcForm * form, QObject *parent = nullptr) :
        GrpcTemplateController(proxyModel,
                               tableView,
                               form,
                               new GrpcObjectWrapper<MasterObject>(),
                               parent){}

    void workerModelData() override {
        //JsonParameterFormatter criterias = searchCriterias();

        // Simulating waiting for a response from the server
        QThread::msleep(500);

        if(masterValid()) {
            QVariant varObject = masterVariantObject();
            MasterObject masterObject = varObject.value<MasterObject>();
            std::vector<SlaveObject> filteredData,
                slaveData = TestModelData::slaveData();

            std::copy_if(slaveData.begin(), slaveData.end(), std::back_inserter(filteredData), [masterObject](SlaveObject & ob) {
                return ob.link_uid() == masterObject.uid();
            });

            emit populateModel(
                std::make_shared<GrpcDataContainer<SlaveObject>>(
                    std::move(filteredData)
                    )
                );
        } else {
            std::vector<SlaveObject> filteredData;
            emit populateModel(
                std::make_shared<GrpcDataContainer<SlaveObject>>(
                    std::move(filteredData)
                    )
                );
        }
    }

    bool masterValid() override {
        QVariant varObject = masterVariantObject();
        MasterObject masterObject = varObject.value<MasterObject>();
        return masterObject.uid() > 0;
    }

    QVariant workerAddNewObject(const QVariant & promise) override {
        // Simulating waiting for a response from the server
        QThread::msleep(300);

        QVariant slaveFormObject = formObject();
        SlaveObject formObject = slaveFormObject.value<SlaveObject>();
        QVariant masterVarObject = masterVariantObject();
        if(!masterVarObject.isValid()) {
            // throw esception
        }
        MasterObject masterObject = masterVarObject.value<MasterObject>();
        formObject.set_link_uid(masterObject.uid());
        formObject.set_uid(randomInt(1000, 100000));

        // We send the object to the server and, if there are no exceptions, return it to the game
        return QVariant::fromValue<SlaveObject>(formObject);
    }
    QVariant workerEditObject(const QVariant & promise) override {
        // Simulating waiting for a response from the server
        QThread::msleep(300);

        // We send the object to the server and, if there are no exceptions, return it to the game
        return promise;
    }
    QVariant workerDeleteObject(const QVariant & promise) override {
        // Simulating waiting for a response from the server
        QThread::msleep(300);

        // We send the object to the server and, if there are no exceptions, return it to the game
        return promise;
    }
};

//Q_DECLARE_METATYPE(MasterObject)
#endif // TESTDATASOURCE_H
