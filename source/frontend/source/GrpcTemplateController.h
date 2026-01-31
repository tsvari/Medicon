#ifndef GRPCTEMPLATECONTROLLER_H
#define GRPCTEMPLATECONTROLLER_H

#include <QObject>
#include <QPointer>
#include <QVariant>
#include <QFutureWatcher>

#include "JsonParameterFormatter.h"

class GrpcForm;
class GrpcProxySortFilterModel;
class GrpcObjectTableModel;
class QAbstractItemView;
class QTabWidget;
class IBaseGrpcObjectWrapper;
class IBaseDataContainer;
class GrpcSearchForm;
class QAction;
class QMenuBar;
class QMenu;
class QToolBar;
class QStatusBar;
class QMainWindow;
class GrpcTableView;
class GrpcLoader;
class GrpcViewNavigator;

/**
 * @brief Base controller for a CRUD-style table + form template.
 *
 * `GrpcTemplateController` coordinates:
 * - A table view (via a proxy model) for row selection
 * - A form for viewing/editing/inserting the current record
 * - Asynchronous loading of model data (via `workerModelData()`)
 *
 * The controller supports master/slave templates:
 * - A *master* controller emits `rowChanged()` as the user changes selection.
 * - A *slave* controller receives master row changes via `masterChanged()` (or
 *   `masterRowChanged` signal wiring) and reloads its own data based on the
 *   current master object.
 *
 * Selection clearing is meaningful:
 * - An invalid `QModelIndex` indicates “no selection / selection cleared”.
 * - Slaves must tolerate the master becoming invalid (e.g. model reset).
 *
 * Threading note:
 * - `startLoadingData()` runs `workerModelData()` using `QtConcurrent::run`.
 * - Derived implementations of `workerModelData()` should ensure that
 *   `populateModel()` is emitted on the controller's thread (e.g. by using
 *   `QMetaObject::invokeMethod(..., Qt::QueuedConnection)`), because emitting
 *   Qt signals from a worker thread can otherwise update UI/models from the
 *   wrong thread.
 */
class GrpcTemplateController : public QObject
{
    Q_OBJECT
public:
    /**
     * @param proxyModel Proxy model set on the table view.
     *                 Its `sourceModel()` is expected to be a `GrpcObjectTableModel`.
     * @param tableView  View that shows the records.
     * @param form       Form bound to the current record (and optionally a master record).
     * @param masterObjectWrapper If non-null, this controller is a slave and uses the
     *                            wrapper to store the currently selected master object.
     */
    explicit GrpcTemplateController(GrpcProxySortFilterModel * proxyModel,
                                    GrpcTableView  * tableView, GrpcForm * form, IBaseGrpcObjectWrapper * masterObjectWrapper, QObject *parent = nullptr);
    virtual ~GrpcTemplateController();

    void addSearchForm(GrpcSearchForm * searchForm);
    void addNavigator(GrpcViewNavigator * navigator);
    virtual void addActionBars(QMainWindow * mainWindow, QMenuBar * menuBar, QToolBar * toolBar, QStatusBar * statusBar);

    enum State {Unselected = 0, Browsing, Edit, Insert};

signals:
    /**
     * @brief Emitted when the current row selection changed.
     *
     * For "no selection", the controller emits an invalid `QModelIndex()`.
     */
    void rowChanged(const QModelIndex & index);

    /**
     * @brief Replaces the underlying data container of the source model.
     *
     * Typically emitted after `workerModelData()` completes.
     */
    void populateModel(std::shared_ptr<IBaseDataContainer> container);

    /**
     * @brief Forwards the master's selected row to a slave controller.
     *
     * Wiring is usually done by `GrpcMasterSlaveController`.
     */
    void masterRowChanged(const QModelIndex & index);

    /**
     * @brief Requests clearing the underlying source model data.
     *
     * This is connected in the constructor to the source model's
     * GrpcObjectTableModel::clearModelData().
     */
    void clearModelDataRequested();

    /**
     * @brief Emitted when this controller is about to start loading model data.
     *
     * This is emitted before launching @ref workerModelData.
     * Useful for dependent controllers (e.g., slave templates) to clear stale data.
     */
    void loadingStarted();

    void showStatusMessage(const QString & message, int timeOut = 0);

    void startInsert();
    void startEdit();
    void finishSave();

    void hideOthers(GrpcTemplateController * controller);

    void makeFormReadonly(bool readOnly);
    void prepareFormObject();

    void addNewObject(const QVariant & data);
    void updateObject(int row, const QVariant & data);
    void deleteObject(int row);

    void warning(const QString & warningTitle, const QString & message);
    void clearForm();
    void clearViewSelection();

    void navigatorRecordCount(int count);

public slots:
    /**
     * @brief Called in a slave template when the master selection changes.
     *
     * If @p index is invalid, the master is considered cleared; the slave clears its
     * own model/selection state and does not attempt to load data.
     */
    virtual void masterChanged(const QModelIndex & index);

    /**
     * @brief Clears the view selection, form and model data.
     *
     * This does not modify the master wrapper (if any).
     */
    void clearModel();

    /**
     * @brief Clears this controller's model when its master is reloading.
     *
     * Clears the table model immediately to avoid stale dependent rows.
     * The form is cleared only when there is a valid master (for slave forms that
     * require a master selection to build their default object).
     */
    void clearForMasterReload();

    void applySearchCriterias( const JsonParameterFormatter & searchCriterias);
    void showMenuAndToolbar();
    void hideMenuAndToolbar();

private slots:
    void currentChanged(const QModelIndex & current, const QModelIndex & previous);
    void formContentChanged();
    void clearSelection();

    void refresh_all();
    void add_new_record();
    void edit_record();
    void delete_record();
    void save_record();
    void escape();

    void handleRefreshGrpc();
    void handleAddNewGrpc();
    void handleEditGrpc();
    void handleDeleteGrpc();

    void receiveFormObject(const QVariant & object);

protected:
    // be sure to override it in the child
    virtual void workerModelData() = 0;

    /**
     * @brief Worker used to add a new object to backend.
     * @param promise Form object payload.
     * @return Backend-confirmed object (e.g. with assigned uid).
     */
    virtual QVariant workerAddNewObject(const QVariant & promise) = 0;

    /**
     * @brief Worker used to edit an object in backend.
     */
    virtual QVariant workerEditObject(const QVariant & promise) = 0;

    /**
     * @brief Worker used to delete an object in backend.
     */
    virtual QVariant workerDeleteObject(const QVariant & promise) = 0;

    // Override in child class for custom states
    virtual void updateState();

    // Check object before edit or insert
    // By default it's valid
    virtual QStringList checkObjectValidity() {return {};}

    // Reload in slave templates
    virtual bool masterValid();

    virtual void startLoadingData();

    QVariant masterVariantObject();
    QVariant & formObject() {return m_formObject;}
    JsonParameterFormatter & searchCriterias();

    QMenu * templateMenu(){return m_templateMenu;}
    QMenu * contextMenu(){return m_contextMenu;}
    QToolBar * templateToolBar();

    int currentNavigatorPage(){return m_currentPage;}
    int maxPages(){return m_maxPages;}

private:
    JsonParameterFormatter m_searchCriterias;
    std::unique_ptr<IBaseGrpcObjectWrapper> m_masterObjectWrapper;

    QVariant m_formObject;
    int m_currentRow = -1;
    State m_state = Unselected;

    QMenu * m_templateMenu = nullptr;
    QMenu * m_contextMenu = nullptr;
    QPointer<QToolBar> m_templateToolBar;
    QPointer<GrpcLoader> m_grpcLoader;

    QAction * m_actionRefresh = nullptr;
    QAction * m_actionAddNew = nullptr;
    QAction * m_actionEdit = nullptr;
    QAction * m_actionDelete = nullptr;
    QAction * m_actionSave = nullptr;

    QFutureWatcher<void> m_watcherLoad;
    QFutureWatcher<QVariant> m_watcherAddNew;
    QFutureWatcher<QVariant> m_watcherEdit;
    QFutureWatcher<QVariant> m_watcherDelete;

    int m_currentPage = -1;
    int m_maxPages = 0;
};

#endif // GRPCTEMPLATECONTROLLER_H
