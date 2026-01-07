#ifndef TESTSHAREDUTILITY_H
#define TESTSHAREDUTILITY_H

#include <QItemSelection>
#include <QVariant>
#include <QRandomGenerator>

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

inline int32_t randomInt(int min, int max) {
    return QRandomGenerator::global()->bounded(max - min + 1) + min;
}

class MasterObject
{
public:
    ~MasterObject(){}
    int32_t uid() const {return m_uid;}
    void set_uid(int32_t value) {m_uid = value;}

    const std::string & name() const {return m_name;}
    void set_name(const std::string & value) {m_name = value;}

    int64_t date() const {return m_date;}
    void set_date(int64_t value) {m_date = value;}

    int64_t time() const {return m_time;}
    void set_time(int64_t value) {m_time = value;}

    int64_t date_time() const {return m_date_time;}
    void set_date_time(int64_t value) {m_date_time = value;}

    int64_t date_time_no_sec() const {return m_date_time_no_sec;}
    void set_date_time_no_sec(int64_t value) {m_date_time_no_sec = value;}

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

    const std::string & married_name() const {return m_married_name;}
    void set_married_name(const std::string & value) {m_married_name = value;}

    const std::string & image() const {return m_image;}
    void set_image(const std::string & value) {m_image = value;}

private:
    int32_t m_uid = 0;
    std::string m_name;
    int64_t m_date = 0;
    int64_t m_time = 0;
    int64_t m_date_time = 0;
    int64_t m_date_time_no_sec = 0;
    int32_t m_height = 0;
    double m_salary = 0.0;
    bool m_married = false;
    std::string m_married_name;
    int32_t m_level = 0; // for combo list
    std::string m_level_name; // combo/edit text
    std::string m_image;
};

class SlaveObject
{
public:
    ~SlaveObject(){}
    SlaveObject(){}
    SlaveObject(int32_t uid, int32_t link_uid, const std::string & phone)
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
    int32_t m_uid = 0;
    int32_t m_link_uid = 0;
    std::string m_phone;
};

#endif // TESTSHAREDUTILITY_H
