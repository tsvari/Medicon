#include "GrpcProxySortFilterModel.h"
#include "GrpcObjectTableModel.h"

GrpcProxySortFilterModel::GrpcProxySortFilterModel(GrpcObjectTableModel * sourceModel, const QList<int> hiddenColumns, QObject * parent)
    : QSortFilterProxyModel(parent)
    , m_hiddenColumns(hiddenColumns)
{
    setSourceModel(sourceModel);
}

bool GrpcProxySortFilterModel::filterAcceptsColumn(int sourceColumn, const QModelIndex & sourceParent) const
{
    return !m_hiddenColumns.contains(sourceColumn);
}
