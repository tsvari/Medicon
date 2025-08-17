#ifndef GRPCUITEMPLATE_H
#define GRPCUITEMPLATE_H

#include <QObject>

#include "JsonParameterFormatter.h"

class GrpcForm;
class GrpcProxySortFilterModel;
class QTableView;
class IBaseGrpcObjectWrapper;
class IBaseDataContainer;
class GrpcUiTemplate : public QObject
{
    Q_OBJECT
public:
    explicit GrpcUiTemplate(GrpcProxySortFilterModel * proxyModel, QTableView * tableView, GrpcForm * form, QObject *parent = nullptr);
    virtual ~GrpcUiTemplate();

signals:
    void rowChanged(const QModelIndex & index);
    void populateModel(IBaseDataContainer * container);

public slots:
    virtual void masterRowChanged(const QModelIndex & index);
    void applySearchCriterias( const JsonParameterFormatter & searchCriterias);

private slots:
    void currentChanged(const QModelIndex & current, const QModelIndex & previous);

protected:
    virtual void modelData() = 0;

    JsonParameterFormatter m_searchCriterias;
    IBaseGrpcObjectWrapper * m_masterObjectWrapper = nullptr;

private:
    GrpcForm * m_form;
    GrpcProxySortFilterModel * m_proxyModel;
    QTableView * m_tableView;    
};

#endif // GRPCUITEMPLATE_H
