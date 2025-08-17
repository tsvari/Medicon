#ifndef GRPCUITEMPLATE_H
#define GRPCUITEMPLATE_H

#include <QObject>

#include "JsonParameterFormatter.h"

class GrpcForm;
class GrpcProxySortFilterModel;
class QAbstractItemView;
class QTabWidget;
class IBaseGrpcObjectWrapper;
class IBaseDataContainer;
class GrpcUiTemplate : public QObject
{
    Q_OBJECT
public:
    explicit GrpcUiTemplate(GrpcProxySortFilterModel * proxyModel, QAbstractItemView  * tableView, GrpcForm * form, IBaseGrpcObjectWrapper * masterObjectWrapper, QObject *parent = nullptr);
    virtual ~GrpcUiTemplate();

signals:
    void rowChanged(const QModelIndex & index);
    void populateModel(IBaseDataContainer * container);

public slots:
    virtual void masterRowChanged(const QModelIndex & index);
    void applySearchCriterias( const JsonParameterFormatter & searchCriterias);

private slots:
    void currentChanged(const QModelIndex & current, const QModelIndex & previous);
    void activateForm();

protected:
    virtual void modelData() = 0;

    QVariant variantObject();
    JsonParameterFormatter & searchCriterias();

private:
    QTabWidget * tabWidget();

    GrpcForm * m_form;
    GrpcProxySortFilterModel * m_proxyModel;
    QAbstractItemView  * m_view;

    JsonParameterFormatter m_searchCriterias;
    IBaseGrpcObjectWrapper * m_masterObjectWrapper = nullptr;
};

#endif // GRPCUITEMPLATE_H
