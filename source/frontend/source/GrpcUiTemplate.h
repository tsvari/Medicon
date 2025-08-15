#ifndef GRPCUITEMPLATE_H
#define GRPCUITEMPLATE_H

#include <QObject>

class GrpcForm;
class GrpcProxySortFilterModel;
class QTableView;
class GrpcUiTemplate : public QObject
{
    Q_OBJECT
public:
    explicit GrpcUiTemplate(GrpcProxySortFilterModel * model, QTableView * table, GrpcForm * form, QObject *parent = nullptr);


signals:
    void sendObject(const QVariant & varData);

private slots:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    GrpcForm * m_form;
    GrpcProxySortFilterModel * m_proxyModel;
    QTableView * m_table;
};

#endif // GRPCUITEMPLATE_H
