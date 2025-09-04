#include "GrpcTemplateController.h"

#include <QAction>
#include <QMainWindow>
#include <QTableView>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>

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

    GrpcObjectTableModel * sourceModel = qobject_cast<GrpcObjectTableModel*>(proxyModel->sourceModel());
    Q_ASSERT(sourceModel);

    sourceModel->initializeModel();
    form->initializeForm();
    form->initilizeWidgets();
    view->setModel(proxyModel);

    connect(this, &GrpcTemplateController::rowChanged, form, &GrpcForm::fill);
    connect(this, &GrpcTemplateController::rowChanged, form, &GrpcForm::selectTab);
    connect(this, &GrpcTemplateController::clearForm, form, &GrpcForm::clear);
    connect(form, &GrpcForm::contentChanged, this, &GrpcTemplateController::updateState);

    connect(this, &GrpcTemplateController::populateModel, sourceModel, &GrpcObjectTableModel::setModelData);
    connect(view->selectionModel(), &QItemSelectionModel::currentChanged, this, &GrpcTemplateController::currentChanged);
    connect(sourceModel, &GrpcObjectTableModel::zerroCount, this, &GrpcTemplateController::clearForm);
    if(masterObjectWrapper) {
        connect(this, &GrpcTemplateController::masterRowChanged, this, &GrpcTemplateController::masterChanged);
    }
}

GrpcTemplateController::~GrpcTemplateController()
{
}

void GrpcTemplateController::addSearchForm(GrpcSearchForm * searchForm)
{
    Q_ASSERT(searchForm);
    connect(searchForm, &GrpcSearchForm::startSearch, this, &GrpcTemplateController::applySearchCriterias);
}

void GrpcTemplateController::addActionBars(QMainWindow * mainWindow, QMenuBar * menuBar, QToolBar * toolBar, QStatusBar * statusBar)
{
    if(menuBar || toolBar) {
        m_actionRefresh = new QAction(tr("&Refresh"), this);
        m_actionRefresh->setIcon(QIcon(":/icons/refresh.png"));
        m_actionRefresh->setShortcut(QKeySequence("F5"));
        m_actionRefresh->setStatusTip(tr("Refresh table"));
        connect(m_actionRefresh, &QAction::triggered, this, &GrpcTemplateController::refresh_all);

        m_actionAddNew = new QAction(tr("Add &New"), this);
        m_actionAddNew->setIcon(QIcon(":/icons/add_new.png"));
        m_actionAddNew->setShortcut(QKeySequence("Ctrl+n"));
        m_actionAddNew->setStatusTip(tr("Add current record"));
        connect(m_actionAddNew, &QAction::triggered, this, &GrpcTemplateController::add_new_record);

        m_actionEdit = new QAction(tr("&Edit"), this);
        m_actionEdit->setIcon(QIcon(":/icons/edit.png"));
        m_actionEdit->setShortcut(QKeySequence("Ctrl+e"));
        m_actionEdit->setStatusTip(tr("Edit current record"));
        connect(m_actionEdit, &QAction::triggered, this, &GrpcTemplateController::edit_record);

        m_actionDelete = new QAction(tr("&Delete"), this);
        m_actionDelete->setIcon(QIcon(":/icons/delete.png"));
        m_actionDelete->setShortcut(QKeySequence("Del"));
        m_actionDelete->setStatusTip(tr("Delete current record"));
        connect(m_actionDelete, &QAction::triggered, this, &GrpcTemplateController::delete_record);

        m_actionSave = new QAction(tr("&Save"), this);
        m_actionSave->setIcon(QIcon(":/icons/save.png"));
        m_actionSave->setShortcut(QKeySequence("Ctrl+s"));
        m_actionSave->setStatusTip(tr("Save Changes"));
        connect(m_actionSave, &QAction::triggered, this, &GrpcTemplateController::save_record);
    }

    if(menuBar) {
        m_templateMenu = menuBar->addMenu(tr("Edit Records"));

        m_templateMenu->addAction(m_actionRefresh);
        m_templateMenu->addSeparator();
        m_templateMenu->addAction(m_actionAddNew);
        m_templateMenu->addAction(m_actionEdit);
        m_templateMenu->addAction(m_actionDelete);
        m_templateMenu->addAction(m_actionSave);
    }

    if(toolBar) {
        m_mainToolBar = toolBar;
        m_templateToolBar = new QToolBar("Template Tool Bar", m_mainToolBar);

        m_templateToolBar->addAction(m_actionRefresh);
        m_templateToolBar->addSeparator();
        m_templateToolBar->addAction(m_actionAddNew);
        m_templateToolBar->addAction(m_actionEdit);
        m_templateToolBar->addAction(m_actionDelete);
        m_templateToolBar->addAction(m_actionSave);

        m_templateToolBar->setIconSize(QSize(16, 16));
        mainWindow->addToolBar(m_templateToolBar);
    }

    if(statusBar) {
        connect(this, &GrpcTemplateController::showStatusMessage, statusBar, &QStatusBar::showMessage);
    }
}

void GrpcTemplateController::masterChanged(const QModelIndex & index)
{
    // Will only be called in the slave template
    // A template can be both a master and a slave at the same time
    QVariant variantObject = index.data(GlobalRoles::VariantObjectRole);
    if(variantObject.isValid()) {
        m_masterObjectWrapper->setObject(variantObject);
        modelData();
        m_state = Unselected;
        m_currentRow = -1;
    }
}

void GrpcTemplateController::applySearchCriterias(const JsonParameterFormatter & searchCriterias)
{
    m_searchCriterias = searchCriterias;
    modelData();
    m_state = Unselected;
    m_currentRow = -1;
}

void GrpcTemplateController::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if(current.row() != previous.row()) {
        emit rowChanged(current);
        m_state = Browsing;
        m_currentRow = current.row();
    }
}

void GrpcTemplateController::updateState()
{
    // Recalculate State
    // Then update actions insert/update/save/delete
}

void GrpcTemplateController::refresh_all()
{

}

void GrpcTemplateController::add_new_record()
{

}

void GrpcTemplateController::edit_record()
{

}

void GrpcTemplateController::delete_record()
{

}

void GrpcTemplateController::save_record()
{

}

QVariant GrpcTemplateController::masterVariantObject()
{
    if(m_masterObjectWrapper) {
        return m_masterObjectWrapper->variantObject();
    }
    return QVariant();
}

JsonParameterFormatter & GrpcTemplateController::searchCriterias()
{
    return m_searchCriterias;
}

