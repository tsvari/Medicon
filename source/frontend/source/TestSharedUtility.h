#ifndef TESTSHAREDUTILITY_H
#define TESTSHAREDUTILITY_H

#include <QItemSelection>
#include <QAbstractItemModel>
#include <QVariant>

#include "GrpcObjectTableModel.h"
#include "TypeToStringFormatter.h"

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

private:
    int32_t m_uid;
    std::string m_name;
    int64_t m_date;
    int32_t m_height;
    double m_salary;
    bool m_married;
    int32_t m_level; // for combo list
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

namespace MasterHeader {
static const  char *  UID      = "Uid";
static const  char *  NAME     = "Name";
static const  char *  DATE     = "Date";
static const  char *  HEIGHT   = "Height";
static const  char *  SALARY   = "Salary";
static const  char *  MARRIED  = "Married";
static const  char *  LEVEL    = "Level";
}
class GrpcTestObjectTableModel : public GrpcObjectTableModel
{
    Q_OBJECT

public:
    explicit GrpcTestObjectTableModel(std::vector<GprcTestDataObject> && data, QObject *parent = nullptr) :
        GrpcObjectTableModel(new GrpcDataContainer<GprcTestDataObject>(std::move(data)), parent)
    {
        initializeData();
    }

    enum COLUMNS {};

    void initializeData() override {
        container()->addProperty(MasterHeader::UID, DataInfo::String, &GprcTestDataObject::set_uid, &GprcTestDataObject::uid);
        container()->addProperty(MasterHeader::NAME, DataInfo::String, &GprcTestDataObject::set_name, &GprcTestDataObject::name);
        container()->addProperty(MasterHeader::DATE, DataInfo::Date, &GprcTestDataObject::set_date, &GprcTestDataObject::date);
        container()->addProperty(MasterHeader::HEIGHT, DataInfo::Int, &GprcTestDataObject::set_height, &GprcTestDataObject::height);
        container()->addProperty(MasterHeader::SALARY, DataInfo::Double, &GprcTestDataObject::set_salary, &GprcTestDataObject::salary);
        container()->addProperty(MasterHeader::MARRIED, DataInfo::Bool, &GprcTestDataObject::set_married, &GprcTestDataObject::married);
        container()->addProperty(MasterHeader::LEVEL, DataInfo::Int, &GprcTestDataObject::set_level, &GprcTestDataObject::level);
        container()->initialize();
    }

private:
    GrpcDataContainer<GprcTestDataObject> * container() {
        return dynamic_cast<GrpcDataContainer<GprcTestDataObject>*>(m_container);
    }

};

namespace SlaveHeadr {
static const  char *  UID      = "Uid";
static const  char *  LINK_UID = "LinkUid";
static const  char *  PHONE    = "Phone";
}
class GrpcTestSlaveObjectTableModel : public GrpcObjectTableModel
{
    Q_OBJECT

public:
    explicit GrpcTestSlaveObjectTableModel(std::vector<GprcTestSlaveObject> && data, QObject *parent = nullptr) :
        GrpcObjectTableModel(new GrpcDataContainer<GprcTestSlaveObject>(std::move(data)), parent)
    {
        initializeData();
    }

    void initializeData() override {
        container()->addProperty(SlaveHeadr::UID, DataInfo::Int, &GprcTestSlaveObject::set_uid, &GprcTestSlaveObject::uid);
        container()->addProperty(SlaveHeadr::LINK_UID, DataInfo::Int, &GprcTestSlaveObject::set_link_uid, &GprcTestSlaveObject::link_uid);
        container()->addProperty(SlaveHeadr::PHONE, DataInfo::String, &GprcTestSlaveObject::set_phone, &GprcTestSlaveObject::phone);
        container()->initialize();
    }

private:
    GrpcDataContainer<GprcTestSlaveObject> * container() {
        return dynamic_cast<GrpcDataContainer<GprcTestSlaveObject>*>(m_container);
    }

};
namespace TestModelData {
static std::vector<GprcTestDataObject> masterData() {
    std::vector<GprcTestDataObject> objects;

    GprcTestDataObject obj1;
    obj1.set_uid(1);
    obj1.set_name("Givi");
    obj1.set_date(TimeFormatHelper::chronoNow().time_since_epoch().count());
    obj1.set_height(168);
    obj1.set_salary(12.15);
    obj1.set_married(false);
    objects.push_back(obj1);

    GprcTestDataObject obj2;
    obj2.set_uid(2);
    obj2.set_name("Keto");
    obj2.set_date(TimeFormatHelper::chronoNow().time_since_epoch().count());
    obj2.set_height(164);
    obj2.set_salary(30.557);
    obj2.set_married(true);
    objects.push_back(obj2);

    GprcTestDataObject obj3;
    obj3.set_uid(3);
    obj3.set_name("Vakho");
    obj3.set_date(TimeFormatHelper::chronoNow().time_since_epoch().count());
    obj3.set_height(175);
    obj3.set_salary(135000.567);
    obj3.set_married(true);
    objects.push_back(obj3);

    GprcTestDataObject obj4;
    obj4.set_uid(4);
    obj4.set_name("Elene");
    obj4.set_date(TimeFormatHelper::chronoNow().time_since_epoch().count());
    obj4.set_height(155);
    obj4.set_salary(567);
    obj4.set_married(false);
    objects.push_back(obj4);

    GprcTestDataObject obj5;
    obj5.set_uid(5);
    obj5.set_name("Teona");
    obj5.set_date(TimeFormatHelper::chronoNow().time_since_epoch().count());
    obj5.set_height(166);
    obj5.set_salary(5.123);
    obj5.set_married(true);
    objects.push_back(obj5);

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
}

Q_DECLARE_METATYPE(GprcTestDataObject)

#endif // TESTSHAREDUTILITY_H
