#include "TestSharedUtility.h"

bool compareQVariant(const QVariant & lhs, const QVariant & rhs) {
    return QVariant::compare(lhs, rhs) == QPartialOrdering::Equivalent;
}

bool compareQVariantList(const QList<QVariant> & lhs, const QList<QVariant> & rhs) {
    if(lhs.size() != rhs.size()) {
        return false;
    }
    QListIterator<QVariant> it1(lhs);
    QListIterator<QVariant> it2(rhs);
    while (it1.hasNext() && it2.hasNext()) {
        if(!compareQVariant(it1.next(), it2.next())) {
            return false;
        }
    }
    return true;
}
