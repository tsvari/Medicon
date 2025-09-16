#include "GrpcTemplateController.h"

#include <QAction>
#include <QMainWindow>
#include <QTableView>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QtConcurrent/QtConcurrent>

#include "GrpcForm.h"
#include "GrpcObjectTableModel.h"
#include "GrpcProxySortFilterModel.h"
#include "GrpcSearchForm.h"
#include "GrpcTableView.h"
#include "GrpcThreadWorker.h"

#include "GrpcDataContainer.hpp"
#include "GrpcObjectWrapper.hpp"

GrpcTemplateController::GrpcTemplateController(GrpcProxySortFilterModel * proxyModel, GrpcThreadWorker * worker,
                                               GrpcTableView  * view, GrpcForm * form, IBaseGrpcObjectWrapper * masterObjectWrapper, QObject *parent)
    : QObject{parent}
    , m_masterObjectWrapper(masterObjectWrapper)
    , m_worker(worker)
{
    Q_ASSERT(proxyModel);
    Q_ASSERT(worker);
    Q_ASSERT(view);
    Q_ASSERT(form);

    m_worker->setParent(this);

    GrpcObjectTableModel * sourceModel = qobject_cast<GrpcObjectTableModel*>(proxyModel->sourceModel());
    Q_ASSERT(sourceModel);

    initActions();
    m_contextMenu = new QMenu(view);
    m_contextMenu->addAction(m_actionRefresh);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_actionAddNew);
    m_contextMenu->addAction(m_actionEdit);
    m_contextMenu->addAction(m_actionDelete);
    m_contextMenu->addAction(m_actionSave);

    connect(view, &GrpcTableView::customContextMenuRequested, this, [this, view](const QPoint & point){
        m_contextMenu->exec(view->mapToGlobal(point));
    });

    sourceModel->initializeModel();
    form->initializeForm();
    form->initilizeWidgets();
    form->makeReadonly(true);
    view->setModel(proxyModel);

    view->addAction(m_actionEscape);

    m_grpcLoader = new GrpcLoader(":/icons/loaderSmall.gif", GrpcLoader::Center, view);
    connect(view, &GrpcTableView::resizeToAdjustLoader, m_grpcLoader, &GrpcLoader::adjustToParentWidget);
    m_grpcLoader->showLoader(false);

    connect(this, &GrpcTemplateController::rowChanged, form, &GrpcForm::fill);
    connect(form, &GrpcForm::formContentChanaged, this, &GrpcTemplateController::formContentChanged);
    connect(form, &GrpcForm::formEscapeSignal, this, &GrpcTemplateController::escape);

    connect(this, &GrpcTemplateController::startInsert, form, &GrpcForm::startInsert);
    connect(this, &GrpcTemplateController::startEdit, form, &GrpcForm::startEdit);
    connect(this, &GrpcTemplateController::finishSave, form, &GrpcForm::finishSave);

    connect(this, &GrpcTemplateController::populateModel, sourceModel, &GrpcObjectTableModel::setModelData);
    connect(view->selectionModel(), &QItemSelectionModel::currentChanged, this, &GrpcTemplateController::currentChanged);
    connect(sourceModel, &GrpcObjectTableModel::zerroCount, this, &GrpcTemplateController::makeFormReadonly);
    connect(this, &GrpcTemplateController::makeFormReadonly, form, &GrpcForm::makeReadonly);
    connect(sourceModel, &GrpcObjectTableModel::zerroCount, this, [this](bool) {m_currentRow = -1;});

    if(masterObjectWrapper) {
        connect(this, &GrpcTemplateController::masterRowChanged, this, &GrpcTemplateController::masterChanged);
    }
    connect(this, &GrpcTemplateController::focusIn, this, [this, view](){
        view->setFocus();
    });
    connect(view, &GrpcTableView::focusIn, form, &GrpcForm::hideAllButThis);
    connect(view, &GrpcTableView::focusIn, this, &GrpcTemplateController::showMenuAndToolbar);
    connect(this, &GrpcTemplateController::prepareFormObject, this, [this, form]() {
        form->fillObject();
        m_formObject = form->object();
        if(!m_formObject.isValid()) {
            // throw exception ????
        }
    });

    connect(this, &GrpcTemplateController::addNewObject, sourceModel, &GrpcObjectTableModel::addNewObject);
    connect(this, &GrpcTemplateController::updateObject, sourceModel, &GrpcObjectTableModel::updateObject);
    connect(this, &GrpcTemplateController::deleteObject, sourceModel, &GrpcObjectTableModel::deleteObject);

    connect(&m_watcherLoad, &QFutureWatcher<IBaseDataContainer *>::finished, this, &GrpcTemplateController::handleRefreshGrpc);
    connect(&m_watcherAddNew, &QFutureWatcher<bool>::finished, this, &GrpcTemplateController::handleAddNewGrpc);
    connect(&m_watcherEdit, &QFutureWatcher<bool>::finished, this, &GrpcTemplateController::handleEditGrpc);
    connect(&m_watcherDelete, &QFutureWatcher<bool>::finished, this, &GrpcTemplateController::handleDeleteGrpc);
}

GrpcTemplateController::~GrpcTemplateController()
{
}

void GrpcTemplateController::addSearchForm(GrpcSearchForm * searchForm)
{
    Q_ASSERT(searchForm);
    connect(searchForm, &GrpcSearchForm::startSearch, this, &GrpcTemplateController::applySearchCriterias);
}

void GrpcTemplateController::initActions()
{
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

    m_actionEscape = new QAction(tr("Escape Action"), this);
    m_actionEscape->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(m_actionEscape, &QAction::triggered, this, &GrpcTemplateController::escape);
    m_actionEscape->setShortcutContext(Qt::WidgetShortcut);
}

void GrpcTemplateController::addActionBars(QMainWindow * mainWindow, QMenuBar * menuBar, QToolBar * toolBar, QStatusBar * statusBar)
{
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
        m_templateToolBar = new QToolBar("Template Tool Bar", toolBar);

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

    hideMenuAndToolbar();
    updateState();
}

void GrpcTemplateController::showMenuAndToolbar()
{
    emit hideOthers(this);
    if(m_templateToolBar->isHidden() || m_templateMenu->isHidden()) {
        m_templateToolBar->setVisible(true);
        m_templateMenu->menuAction()->setVisible(true);
    }
}


void GrpcTemplateController::hideMenuAndToolbar()
{
    if(m_templateToolBar->isVisible() || m_templateMenu->isVisible()) {
        m_templateToolBar->setVisible(false);
        m_templateMenu->menuAction()->setVisible(false);
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
    m_currentRow = -1;
    m_state = Unselected;
    updateState();
}


void GrpcTemplateController::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if(current.row() != previous.row()) {
        emit rowChanged(current);
        if(m_state != Browsing) {
            emit finishSave();
        }
        m_currentRow = current.row();
        m_state = Browsing;
        updateState();
    }
}

void GrpcTemplateController::formContentChanged()
{
    if(m_state == Browsing) {
        m_state = Edit;
        updateState();
        emit startEdit();
    }
}

void GrpcTemplateController::updateState()
{
    // Recalculate State
    // Then update actions insert/update/save/delete
    switch(m_state) {
    case Unselected:
        m_actionRefresh->setEnabled(true);

        m_actionAddNew->setEnabled(true);
        m_actionEdit->setEnabled(false);
        m_actionDelete->setEnabled(false);
        m_actionSave->setEnabled(false);
        break;
    case Browsing:
        m_actionRefresh->setEnabled(true);
        m_actionAddNew->setEnabled(true);
        m_actionEdit->setEnabled(true);
        m_actionDelete->setEnabled(true);
        m_actionSave->setEnabled(false);
        break;
    case Edit:
        m_actionRefresh->setEnabled(true);
        m_actionAddNew->setEnabled(false);
        m_actionEdit->setEnabled(false);
        m_actionDelete->setEnabled(false);
        m_actionSave->setEnabled(true);
        break;
    case Insert:
        m_actionRefresh->setEnabled(true);
        m_actionAddNew->setEnabled(false);
        m_actionEdit->setEnabled(false);
        m_actionDelete->setEnabled(false);
        m_actionSave->setEnabled(true);
        break;
    }
}

void GrpcTemplateController::refresh_all()
{
    m_grpcLoader->showLoader(true);
    QFuture<IBaseDataContainer *> future = QtConcurrent::run(&GrpcThreadWorker::loadObjects, m_worker);
    m_watcherLoad.setFuture(future);
}

void GrpcTemplateController::add_new_record()
{
    m_state = Insert;
    emit startInsert();
    emit makeFormReadonly(false);
    updateState();
}

void GrpcTemplateController::edit_record()
{
    m_state = Edit;
    emit startEdit();
    updateState();
}

void GrpcTemplateController::delete_record()
{
    emit prepareFormObject();
    m_grpcLoader->showLoader(true);
    QFuture<void> future = QtConcurrent::run(&GrpcThreadWorker::deleteObject, m_worker, m_formObject);
    m_watcherDelete.setFuture(future);
}

void GrpcTemplateController::handleRefreshGrpc()
{
    try {
        IBaseDataContainer * result = m_watcherLoad.result();
        // refresh means connect to server and request data based on existing search criterias
        modelData();
        //emit populateModel(std::make_shared<IBaseDataContainer>(*result));
        m_currentRow = -1;
        m_state = Unselected;
        updateState();
        m_grpcLoader->showLoader(false);
    } catch (const QUnhandledException & e) {

    }
}

void GrpcTemplateController::handleAddNewGrpc()
{
     try {

        // sent Grpc object to server to add it and if success add it to the model
        emit addNewObject(m_formObject);
        m_state = Browsing;
        updateState();
        emit finishSave();
        m_grpcLoader->showLoader(false);
     } catch (const QUnhandledException & e) {

     }
}

void GrpcTemplateController::handleEditGrpc()
{
    try {

        // sent Grpc object to server to edit record and if success edit current object in the model
        emit updateObject(m_currentRow, m_formObject);
        m_state = Browsing;
        updateState();
        emit finishSave();
        m_grpcLoader->showLoader(false);
    } catch (const QUnhandledException & e) {

    }
}

void GrpcTemplateController::handleDeleteGrpc()
{
    try {

        // send Grpc object to server delete it and if success remove it from the model
        emit deleteObject(m_currentRow);
        m_grpcLoader->showLoader(false);
    } catch (const QUnhandledException & e) {

    }
}

void GrpcTemplateController::save_record()
{
    if(m_state == Edit) {
        emit prepareFormObject();
        m_grpcLoader->showLoader(true);
        QFuture<void> future = QtConcurrent::run(&GrpcThreadWorker::editObject, m_worker, m_formObject);
        m_watcherEdit.setFuture(future);
    } else if(m_state == Insert) {
        emit prepareFormObject();
        m_grpcLoader->showLoader(true);
        QFuture<void> future = QtConcurrent::run(&GrpcThreadWorker::addNewObject, m_worker, m_formObject);
        m_watcherAddNew.setFuture(future);
    }
}

void GrpcTemplateController::escape()
{
    if(m_state == Insert || m_state == Edit) {
        if(m_currentRow >= 0) {
            m_state = Browsing;
        } else {
            m_state = Unselected;
            emit makeFormReadonly(true);
        }
        updateState();
        emit finishSave();
    }
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



