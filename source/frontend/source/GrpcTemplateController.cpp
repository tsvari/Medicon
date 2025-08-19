#include "GrpcTemplateController.h"

#include <QTableView>

#include "GrpcForm.h"
#include "GrpcObjectTableModel.h"
#include "GrpcProxySortFilterModel.h"
#include "GrpcSearchForm.h"

#include "GrpcDataContainer.hpp"
#include "GrpcObjectWrapper.hpp"

GrpcTemplateController::GrpcTemplateController(GrpcProxySortFilterModel * proxyModel, QAbstractItemView  * view, GrpcForm * form, IBaseGrpcObjectWrapper * masterObjectWrapper, QObject *parent)
    : QObject{parent}
    , m_masterObjectWrapper(masterObjectWrapper)
{
    Q_ASSERT(proxyModel);
    Q_ASSERT(view);
    Q_ASSERT(form);
    Q_ASSERT(masterObjectWrapper);

    GrpcObjectTableModel * sourceModel = qobject_cast<GrpcObjectTableModel*>(proxyModel->sourceModel());
    Q_ASSERT(sourceModel);

    sourceModel->initializeModel();
    form->initializeData();
    view->setModel(proxyModel);

    connect(this, &GrpcTemplateController::rowChanged, form, &GrpcForm::fillForm);
    connect(this, &GrpcTemplateController::populateModel, sourceModel, &GrpcObjectTableModel::setModelData);
    connect(view->selectionModel(), &QItemSelectionModel::currentChanged, this, &GrpcTemplateController::currentChanged);
    connect(sourceModel, &GrpcObjectTableModel::zerroCount, this, [form]() {
        form->clearForm();
    });

    auto getTabWidget = [=]() -> QTabWidget* {
        QWidget * currentParent = form->parentWidget();
        while (currentParent) {
            if (QTabWidget * tabWidget = qobject_cast<QTabWidget *>(currentParent)) {
                return tabWidget;
            }
            currentParent = currentParent->parentWidget();
        }
        return nullptr;
    };
    connect(this, &GrpcTemplateController::rowChanged, this, [getTabWidget, form]() {
        if(QTabWidget * widget = getTabWidget()) {
            // m_form->parentWidget() should be QWidget (Tab)
            // Do not place an extra layer in the form
            if(widget->currentWidget() != form->parentWidget()) {
                widget->setCurrentWidget(form->parentWidget());
            }
        }
    });
}

GrpcTemplateController::~GrpcTemplateController()
{
    if(m_masterObjectWrapper) {
        delete m_masterObjectWrapper;
    }
}

void GrpcTemplateController::addSearchForm(GrpcSearchForm * searchForm)
{
    Q_ASSERT(searchForm);
    connect(searchForm, &GrpcSearchForm::startSearch, this, &GrpcTemplateController::applySearchCriterias);
}

void GrpcTemplateController::masterRowChanged(const QModelIndex & index)
{
    // Will only be called in the slave template
    // A template can be both a master and a slave at the same time
    QVariant variantObject = index.data(GlobalRoles::VariantObjectRole);
    if(variantObject.isValid()) {
        m_masterObjectWrapper->setObject(variantObject);
        modelData();
    }
}

void GrpcTemplateController::applySearchCriterias(const JsonParameterFormatter & searchCriterias)
{
    m_searchCriterias = searchCriterias;
    modelData();
}

void GrpcTemplateController::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if(current.row() != previous.row()) {
        emit rowChanged(current);
    }
}

QVariant GrpcTemplateController::variantObject()
{
    return m_masterObjectWrapper->variantObject();
}

JsonParameterFormatter & GrpcTemplateController::searchCriterias()
{
    return m_searchCriterias;
}

