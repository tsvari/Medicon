#ifndef GRPCPROXYSORTFILTERMODEL_H
#define GRPCPROXYSORTFILTERMODEL_H

#include <QSortFilterProxyModel >

class GrpcObjectTableModel;
class GrpcProxySortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit GrpcProxySortFilterModel(GrpcObjectTableModel * sourceModel, const QList<int> hiddenColumns, QObject * parent = nullptr);

    bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const override;

protected:
    QList<int> m_hiddenColumns;
};

#endif // GRPCPROXYSORTFILTERMODEL_H
