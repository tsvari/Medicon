#ifndef TESTSHAREDUTILITY_H
#define TESTSHAREDUTILITY_H

#include <QItemSelection>
#include <QModelIndex>
#include <QVariant>

template <typename TpT = QVariant>
auto pullout(const QItemSelection & sel, std::function<TpT(QModelIndex)> & get_type) {
    std::vector<TpT> results;
    for(const auto & index: sel.indexes()) {
        results.push_back(get_type(index));
    }
}

template <typename TpT = QVariant>
auto pullout(const QItemSelection & sel, int role) {
    std::function func = [role](QModelIndex index) {
        return index.data(role).value<TpT>();
    };
    return pullout<TpT>(sel, func);
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


#endif // TESTSHAREDUTILITY_H
