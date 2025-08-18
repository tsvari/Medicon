#ifndef GRPCTEMPLATECONTROLLER_H
#define GRPCTEMPLATECONTROLLER_H

#include <QObject>

#include "JsonParameterFormatter.h"

class GrpcForm;
class GrpcProxySortFilterModel;
class QAbstractItemView;
class QTabWidget;
class IBaseGrpcObjectWrapper;
class IBaseDataContainer;
class GrpcSearchForm;
class GrpcTemplateController : public QObject
{
    Q_OBJECT
public:
    explicit GrpcTemplateController(GrpcProxySortFilterModel * proxyModel, QAbstractItemView  * tableView, GrpcForm * form, IBaseGrpcObjectWrapper * masterObjectWrapper, QObject *parent = nullptr);
    virtual ~GrpcTemplateController();

    void addSearchForm(GrpcSearchForm * searchForm);

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

    QVariant variantObject();
    JsonParameterFormatter & searchCriterias();

private:
    JsonParameterFormatter m_searchCriterias;
    IBaseGrpcObjectWrapper * m_masterObjectWrapper = nullptr;
};

#endif // GRPCTEMPLATECONTROLLER_H
