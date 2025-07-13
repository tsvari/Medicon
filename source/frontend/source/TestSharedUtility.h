#ifndef TESTSHAREDUTILITY_H
#define TESTSHAREDUTILITY_H

#include <QItemSelection>
#include <QAbstractItemModel>
#include <QVariant>

#include "GrpcObjectTableModel.h"

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

private:
    std::string m_name;
    int64_t m_date;
    int32_t m_height;
    double m_salary;
    bool m_married;
};

class GrpcTestObjectTableModel : public GrpcObjectTableModel
{
    Q_OBJECT

public:
    explicit GrpcTestObjectTableModel(std::vector<GprcTestDataObject> && data, QObject *parent = nullptr) :
        GrpcObjectTableModel(new GrpcDataContainer<GprcTestDataObject>(std::move(data)), parent)
    {
        initializeData();
    }

    void initializeData() override {
        container()->addProperty("Name", DataInfo::String, &GprcTestDataObject::set_name, &GprcTestDataObject::name);
        container()->addProperty("Date", DataInfo::Date, &GprcTestDataObject::set_date, &GprcTestDataObject::date);
        container()->addProperty("Height", DataInfo::Int, &GprcTestDataObject::set_height, &GprcTestDataObject::height);
        container()->addProperty("Salary", DataInfo::Double, &GprcTestDataObject::set_salary, &GprcTestDataObject::salary);
        container()->addProperty("Married", DataInfo::Bool, &GprcTestDataObject::set_married, &GprcTestDataObject::married);
        container()->initialize();
    }

private:
    GrpcDataContainer<GprcTestDataObject> * container() {
        return dynamic_cast<GrpcDataContainer<GprcTestDataObject>*>(m_container);
    }

};

Q_DECLARE_METATYPE(GprcTestDataObject)

#endif // TESTSHAREDUTILITY_H
