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

    enum State {Unselected = 0, Browsing, Edit, Insert};

signals:
    void rowChanged(const QModelIndex & index);
    void populateModel(std::shared_ptr<IBaseDataContainer> container);
    void masterRowChanged(const QModelIndex & index);
    void clearForm();

public slots:
    virtual void masterChanged(const QModelIndex & index);
    void applySearchCriterias( const JsonParameterFormatter & searchCriterias);

private slots:
    void currentChanged(const QModelIndex & current, const QModelIndex & previous);

protected:
    // be sure to override it in the child
    virtual void modelData() = 0;

    QVariant masterVariantObject();
    JsonParameterFormatter & searchCriterias();

private:
    JsonParameterFormatter m_searchCriterias;
    std::unique_ptr<IBaseGrpcObjectWrapper> m_masterObjectWrapper;
    int m_currentRow = -1;
    State m_state = Unselected;
};

#endif // GRPCTEMPLATECONTROLLER_H
