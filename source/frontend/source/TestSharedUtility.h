#ifndef TESTSHAREDUTILITY_H
#define TESTSHAREDUTILITY_H

#include <QItemSelection>
#include <QAbstractItemModel>
#include <QVariant>
#include <QLineEdit>
#include <QTableView>
#include <QToolBar>
#include <QMenuBar>
#include <QMenu>

#include "GrpcObjectTableModel.h"
#include "GrpcDataContainer.hpp"
#include "TypeToStringFormatter.h"
#include "GrpcForm.h"
#include "GrpcProxySortFilterModel.h"
#include "GrpcTemplateController.h"
#include "include_frontend_util.h"

template <typename TpT = QVariant>
auto pullout(const QItemSelection & sel, std::function<TpT(QModelIndex)> & get_type) {
    std::vector<TpT> results;
    for(const auto & index: sel.indexes()) {
        results.push_back(get_type(index));
    }
    return results;
}

template <typename TpT = QVariant>
auto pullout(const QItemSelection & sel, int role) {
    std::function func = [role](QModelIndex index) {
        return index.data(role).value<TpT>();
    };
    return pullout<TpT>(sel, func);
}

template <typename TpT = QVariant>
auto pulloutHeader(QAbstractItemModel * model, const std::vector<int> & sections, Qt::Orientation orientation, int role) {
    std::vector<TpT> results;
    for(const auto & section: sections) {
        results.push_back(model->headerData(section, orientation, role).value<TpT>());
    }
    return results;
}


bool compareQVariant(const QVariant & lhs, const QVariant & rhs);
bool compareQVariantList(const QList<QVariant> & lhs, const QList<QVariant> & rhs);

template<typename T>
const T& deref_if_refwrap(const T& val) {
    return val;
}

template<typename T>
const T& deref_if_refwrap(const std::reference_wrapper<T>& ref) {
    return ref.get();
}

template<typename... Types>
bool loose_compare(const std::variant<Types...>& a, const std::variant<Types...>& b) {
    return std::visit([](const auto& lhs_raw, const auto& rhs_raw) -> bool {
        const auto& lhs = deref_if_refwrap(lhs_raw);
        const auto& rhs = deref_if_refwrap(rhs_raw);

        using L = std::decay_t<decltype(lhs)>;
        using R = std::decay_t<decltype(rhs)>;

        if constexpr (std::is_same_v<L, R>) {
            return lhs == rhs;
        }
        else if constexpr (std::is_arithmetic_v<L> && std::is_arithmetic_v<R>) {
            //return lhs == rhs;
            using Common = std::common_type_t<L, R>;
            return static_cast<Common>(lhs) == static_cast<Common>(rhs);
        }
        else {
            return false;
        }
    }, a, b);
}

// Compare two vectors of variants
template<typename... Types>
bool loose_vector_compare(const std::vector<std::variant<Types...>>& v1,
                          const std::vector<std::variant<Types...>>& v2) {
    if (v1.size() != v2.size())
        return false;

    for (size_t i = 0; i < v1.size(); ++i) {
        if (!loose_compare(v1[i], v2[i]))
            return false;
    }

    return true;
}

class GprcTestDataObject
{
public:
    ~GprcTestDataObject(){}
    int32_t uid() const {return m_uid;}
    void set_uid(int32_t value) {m_uid = value;}

    const std::string & name() const {return m_name;}
    void set_name(const std::string & value) {m_name = value;}

    int64_t date() const {return m_date;}
    void set_date(int64_t value) {m_date = value;}

    int32_t height() const {return m_height;}
    void set_height(int32_t value) {m_height = value;}

    double salary() const {return m_salary;}
    void set_salary(double value) {m_salary = value;}

    bool married() const {return m_married;}
    void set_married(bool value) {m_married = value;}

    int32_t level() const {return m_level;}
    void set_level(int32_t value) {m_level = value;}

    const std::string & level_name() const {return m_level_name;}
    void set_level_name(const std::string & value) {m_level_name = value;}

private:
    int32_t m_uid;
    std::string m_name;
    int64_t m_date;
    int32_t m_height;
    double m_salary;
    bool m_married;
    int32_t m_level; // for combo list
    std::string m_level_name; // combo/edit text
};

class GprcTestSlaveObject
{
public:
    ~GprcTestSlaveObject(){}
    GprcTestSlaveObject(){}
    GprcTestSlaveObject(int32_t uid, int32_t link_uid, const std::string & phone)
        : m_uid(uid)
        , m_link_uid(link_uid)
        , m_phone(phone)
    {
    }

    int32_t uid() const {return m_uid;}
    void set_uid(int32_t value) {m_uid = value;}

    int32_t link_uid() const {return m_link_uid;}
    void set_link_uid(int32_t value) {m_link_uid = value;}

    const std::string & phone() const {return m_phone;}
    void set_phone(const std::string & value) {m_phone = value;}

private:
    int32_t m_uid;
    int32_t m_link_uid;
    std::string m_phone;
};

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

class GrpcTestObjectTableModel : public GrpcObjectTableModel
{
    Q_OBJECT

public:
    explicit GrpcTestObjectTableModel(std::vector<GprcTestDataObject> && data, QObject *parent = nullptr) :
        GrpcObjectTableModel(new GrpcDataContainer<GprcTestDataObject>(std::move(data)), parent)
    {
        initializeModel();
        initializeContainer();
    }

    explicit GrpcTestObjectTableModel(QObject *parent = nullptr) :
        GrpcObjectTableModel( new GrpcDataContainer<GprcTestDataObject>(), parent)
    {
    }

    void initializeModel() override {
        GrpcDataContainer<GprcTestDataObject> * container = dynamic_cast<GrpcDataContainer<GprcTestDataObject>*>(objectContainer());

        container->addProperty("Uid", DataInfo::String, &GprcTestDataObject::set_uid, &GprcTestDataObject::uid);
        container->addProperty("Name", DataInfo::String, &GprcTestDataObject::set_name, &GprcTestDataObject::name);
        container->addProperty("Date", DataInfo::Date, &GprcTestDataObject::set_date, &GprcTestDataObject::date);
        container->addProperty("Height", DataInfo::Int, &GprcTestDataObject::set_height, &GprcTestDataObject::height);
        container->addProperty("Salary", DataInfo::Double, &GprcTestDataObject::set_salary, &GprcTestDataObject::salary);
        container->addProperty("Married", DataInfo::Bool, &GprcTestDataObject::set_married, &GprcTestDataObject::married);
        container->addProperty("Level", DataInfo::Int, &GprcTestDataObject::set_level, &GprcTestDataObject::level);
        container->addProperty("Level Name", DataInfo::String, &GprcTestDataObject::set_level_name, &GprcTestDataObject::level_name);
    }
};

class GrpcTestSlaveObjectTableModel : public GrpcObjectTableModel
{
    Q_OBJECT

public:
    explicit GrpcTestSlaveObjectTableModel(std::vector<GprcTestSlaveObject> && data, QObject *parent = nullptr) :
        GrpcObjectTableModel(new GrpcDataContainer<GprcTestSlaveObject>(std::move(data)), parent)
    {
        initializeModel();
        initializeContainer();
    }

    explicit GrpcTestSlaveObjectTableModel(QObject *parent = nullptr) :
        GrpcObjectTableModel(new GrpcDataContainer<GprcTestSlaveObject>(), parent)
    {
    }

    void initializeModel() override {
        GrpcDataContainer<GprcTestSlaveObject> * container = dynamic_cast<GrpcDataContainer<GprcTestSlaveObject>*>(objectContainer());

        container->addProperty("Uid", DataInfo::Int, &GprcTestSlaveObject::set_uid, &GprcTestSlaveObject::uid);
        container->addProperty("LinkUid", DataInfo::Int, &GprcTestSlaveObject::set_link_uid, &GprcTestSlaveObject::link_uid);
        container->addProperty("Phone", DataInfo::String, &GprcTestSlaveObject::set_phone, &GprcTestSlaveObject::phone);
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
        : GrpcForm( new GrpcObjectWrapper<GprcTestDataObject>(), parent){
    }

    void initializeForm() override {
        GrpcObjectWrapper<GprcTestDataObject> * wrapper = dynamic_cast<GrpcObjectWrapper<GprcTestDataObject>*>(objectWrapper());

        //wrapper()->addProperty("Uid", DataInfo::String, &GprcTestDataObject::set_uid, &GprcTestDataObject::uid);
        wrapper->addProperty("nameEdit", DataInfo::String, &GprcTestDataObject::set_name, &GprcTestDataObject::name);
        wrapper->addProperty("dateEdit", DataInfo::Date, &GprcTestDataObject::set_date, &GprcTestDataObject::date);
        wrapper->addProperty("heightEdit", DataInfo::Int, &GprcTestDataObject::set_height, &GprcTestDataObject::height);
        wrapper->addProperty("salaryEdit", DataInfo::Double, &GprcTestDataObject::set_salary, &GprcTestDataObject::salary);
        wrapper->addProperty("marriedCheckBox", DataInfo::Bool, &GprcTestDataObject::set_married, &GprcTestDataObject::married);
        wrapper->addProperty("levelCombo", DataInfo::Int, &GprcTestDataObject::set_level, &GprcTestDataObject::level);
        //wrapper->addProperty("levelCombo", DataInfo::String, &GprcTestDataObject::set_level, &GprcTestDataObject::level);
    }
};

class SlaveForm : public GrpcForm
{
    Q_OBJECT

public:
    explicit SlaveForm(QWidget *parent = nullptr) :
        GrpcForm(new GrpcObjectWrapper<GprcTestSlaveObject>(), parent){

    }

    void initializeForm() override {
        GrpcObjectWrapper<GprcTestSlaveObject> * wrapper = dynamic_cast<GrpcObjectWrapper<GprcTestSlaveObject>*>(objectWrapper());
        //wrapper->addProperty("Uid", DataInfo::Int, &GprcTestSlaveObject::set_uid, &GprcTestSlaveObject::uid);
        //wrapper->addProperty("LinkUid", DataInfo::Int, &GprcTestSlaveObject::set_link_uid, &GprcTestSlaveObject::link_uid);
        wrapper->addProperty("phoneEdit", DataInfo::String, &GprcTestSlaveObject::set_phone, &GprcTestSlaveObject::phone);
    }
};

namespace TestModelData {
static std::vector<GprcTestDataObject> masterData() {
    std::vector<GprcTestDataObject> objects;

    static int incr = 0;
    auto increament = [=]() {
        incr += 11111;
        return (TimeFormatHelper::chronoNow().time_since_epoch().count() + incr);
    };

    GprcTestDataObject obj1;
    obj1.set_uid(1);
    obj1.set_name("Givi");
    obj1.set_date(increament());
    obj1.set_height(168);
    obj1.set_salary(12.15);
    obj1.set_married(false);
    obj1.set_level(2);
    obj1.set_level_name("Level2");
    objects.push_back(obj1);

    GprcTestDataObject obj2;
    obj2.set_uid(2);
    obj2.set_name("Keto");
    obj2.set_date(increament());
    obj2.set_height(164);
    obj2.set_salary(30.557);
    obj2.set_married(true);
    obj2.set_level(1);
    obj2.set_level_name("Level1");
    objects.push_back(obj2);

    GprcTestDataObject obj3;
    obj3.set_uid(3);
    obj3.set_name("Vakho");
    obj3.set_date(increament());
    obj3.set_height(175);
    obj3.set_salary(135000.567);
    obj3.set_married(true);
    obj3.set_level(3);
    obj3.set_level_name("Level3");
    objects.push_back(obj3);

    GprcTestDataObject obj4;
    obj4.set_uid(4);
    obj4.set_name("Elene");
    obj4.set_date(increament());
    obj4.set_height(155);
    obj4.set_salary(567);
    obj4.set_married(false);
    obj4.set_level(5);
    obj4.set_level_name("Level5");
    objects.push_back(obj4);

    GprcTestDataObject obj5;
    obj5.set_uid(5);
    obj5.set_name("Teona");
    obj5.set_date(increament());
    obj5.set_height(166);
    obj5.set_salary(5.123);
    obj5.set_married(true);
    obj5.set_level(4);
    obj5.set_level_name("Level4");
    objects.push_back(obj5);

    GprcTestDataObject obj6;
    obj6.set_uid(6);
    obj6.set_name("Tsio");
    obj6.set_date(increament());
    obj6.set_height(166);
    obj6.set_salary(5.123);
    obj6.set_married(true);
    obj6.set_level(4);
    obj6.set_level_name("Level4");
    objects.push_back(obj6);

    return objects;
}

static std::vector<GprcTestSlaveObject> slaveData()
{
    std::vector<GprcTestSlaveObject> objects {
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

class MasterTemplate : public GrpcTemplateController
{
    Q_OBJECT

public: explicit MasterTemplate(GrpcProxySortFilterModel * model, GrpcTableView * tableView, GrpcForm * form, QObject *parent = nullptr) :
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
    }

    void updateState() override {
        // Calculate state for generic bar and menu
        GrpcTemplateController::updateState();

        // More enable/disable custom actions

    }

    void modelData() override {
        //JsonParameterFormatter criterias = searchCriterias();
        // Dont need master object in thinscase

        emit populateModel(
            std::make_shared<GrpcDataContainer<GprcTestDataObject>>(
                std::move(TestModelData::masterData())
                )
            );
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
                         new GrpcObjectWrapper<GprcTestDataObject>(),
                         parent){}

    void modelData() override {
        //JsonParameterFormatter criterias = searchCriterias();
        QVariant varObject = masterVariantObject();
        if(varObject.isValid()) {
            GprcTestDataObject masterObject = varObject.value<GprcTestDataObject>();
            std::vector<GprcTestSlaveObject> filteredData,
                                             slaveData = TestModelData::slaveData();

            std::copy_if(slaveData.begin(), slaveData.end(), std::back_inserter(filteredData), [masterObject](GprcTestSlaveObject & ob) {
                return ob.link_uid() == masterObject.uid();
            });

            emit populateModel(
                std::make_shared<GrpcDataContainer<GprcTestSlaveObject>>(
                    std::move(filteredData)
                    )
                );
        }
    }
};

Q_DECLARE_METATYPE(GprcTestDataObject)

#endif // TESTSHAREDUTILITY_H
