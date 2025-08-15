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
    explicit GrpcUiTemplate(GrpcProxySortFilterModel * model, QTableView * tableView, GrpcForm * form, QObject *parent = nullptr);

signals:
    void rowChanged(const QModelIndex & index);

public slots:
    void masterRowChanged(const QModelIndex & index);

private slots:
    void currentChanged(const QModelIndex & current, const QModelIndex & previous);

private:
    GrpcForm * m_form;
    GrpcProxySortFilterModel * m_proxyModel;
    QTableView * m_tableView;
};

#endif // GRPCUITEMPLATE_H
