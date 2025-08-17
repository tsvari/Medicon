#include "GrpcUiTemplate.h"

#include <QTableView>

#include "GrpcForm.h"
#include "GrpcObjectTableModel.h"
#include "GrpcProxySortFilterModel.h"

#include "GrpcDataContainer.hpp"
#include "GrpcObjectWrapper.hpp"

GrpcUiTemplate::GrpcUiTemplate(GrpcProxySortFilterModel * proxyModel, QTableView * tableView, GrpcForm * form, QObject *parent)
    : QObject{parent}
    , m_proxyModel(proxyModel)
    , m_tableView(tableView)
    , m_form(form)
{
    Q_ASSERT(proxyModel);
    Q_ASSERT(tableView);
    Q_ASSERT(form);

    GrpcObjectTableModel * sourceModel = qobject_cast<GrpcObjectTableModel*>(m_proxyModel->sourceModel());
    Q_ASSERT(sourceModel);

    sourceModel->initializeData();
    form->initializeData();
    tableView->setModel(proxyModel);

    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(this, &GrpcUiTemplate::rowChanged, form, &GrpcForm::fillForm);
    connect(this, &GrpcUiTemplate::populateModel, sourceModel, &GrpcObjectTableModel::setModelData);
    connect(tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &GrpcUiTemplate::currentChanged);
}

GrpcUiTemplate::~GrpcUiTemplate()
{
    if(m_masterObjectWrapper) {
        delete m_masterObjectWrapper;
    }
}

void GrpcUiTemplate::masterRowChanged(const QModelIndex & index)
{
    // Does nothing by default
    // Override in you slave template
    // A template can be both a master and a slave at the same time

    // ToDO: In slave template initilize m_masterObjectWrapper with QVariant from index
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
