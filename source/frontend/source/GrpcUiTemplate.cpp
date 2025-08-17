#include "GrpcUiTemplate.h"

#include <QTableView>

#include "GrpcForm.h"
#include "GrpcObjectTableModel.h"
#include "GrpcProxySortFilterModel.h"

#include "GrpcDataContainer.hpp"
#include "GrpcObjectWrapper.hpp"

GrpcUiTemplate::GrpcUiTemplate(GrpcProxySortFilterModel * proxyModel, QAbstractItemView  * view, GrpcForm * form, IBaseGrpcObjectWrapper * masterObjectWrapper, QObject *parent)
    : QObject{parent}
    , m_proxyModel(proxyModel)
    , m_view(view)
    , m_form(form)
    , m_masterObjectWrapper(masterObjectWrapper)
{
    Q_ASSERT(proxyModel);
    Q_ASSERT(view);
    Q_ASSERT(form);

    GrpcObjectTableModel * sourceModel = qobject_cast<GrpcObjectTableModel*>(m_proxyModel->sourceModel());
    Q_ASSERT(sourceModel);

    sourceModel->initializeData();
    form->initializeData();
    view->setModel(proxyModel);

    connect(this, &GrpcUiTemplate::rowChanged, form, &GrpcForm::fillForm);
    connect(this, &GrpcUiTemplate::populateModel, sourceModel, &GrpcObjectTableModel::setModelData);
    connect(view->selectionModel(), &QItemSelectionModel::currentChanged, this, &GrpcUiTemplate::currentChanged);
    connect(this, &GrpcUiTemplate::rowChanged, this, &GrpcUiTemplate::activateForm);
}

GrpcUiTemplate::~GrpcUiTemplate()
{
    if(m_masterObjectWrapper) {
        delete m_masterObjectWrapper;
    }
}

void GrpcUiTemplate::masterRowChanged(const QModelIndex & index)
{
    // Will only be called in the slave template
    // A template can be both a master and a slave at the same time
    QVariant variantObject = index.data(GlobalRoles::VariantObjectRole);
    if(variantObject.isValid()) {
        m_masterObjectWrapper->setObject(variantObject);
        modelData();
    }
}

void GrpcUiTemplate::applySearchCriterias(const JsonParameterFormatter & searchCriterias)
{
    m_searchCriterias = searchCriterias;
    modelData();
}

void GrpcUiTemplate::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if(current.row() != previous.row()) {
        emit rowChanged(current);
    }
}

void GrpcUiTemplate::activateForm()
{
    if(QTabWidget * widget = tabWidget()) {
        // m_form->parentWidget() should be QWidget (Tab)
        // Do not place an extra layer in the form
        if(widget->currentWidget() != m_form->parentWidget()) {
            widget->setCurrentWidget(m_form->parentWidget());
        }
    }
}

QVariant GrpcUiTemplate::variantObject()
{
    return m_masterObjectWrapper->variantObject();
}

JsonParameterFormatter & GrpcUiTemplate::searchCriterias()
{
    return m_searchCriterias;
}

QTabWidget * GrpcUiTemplate::tabWidget()
{
    QWidget * currentParent = m_form->parentWidget();
    while (currentParent) {
        if (QTabWidget * tabWidget = qobject_cast<QTabWidget *>(currentParent)) {
            return tabWidget;
        }
        currentParent = currentParent->parentWidget();
    }
    return nullptr;
}
