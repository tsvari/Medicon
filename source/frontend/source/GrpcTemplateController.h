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
class QAction;
class QMenuBar;
class QMenu;
class QToolBar;
class QStatusBar;
class QMainWindow;
class GrpcTableView;
class GrpcTemplateController : public QObject
{
    Q_OBJECT
public:
    explicit GrpcTemplateController(GrpcProxySortFilterModel * proxyModel, GrpcTableView  * tableView, GrpcForm * form, IBaseGrpcObjectWrapper * masterObjectWrapper, QObject *parent = nullptr);
    virtual ~GrpcTemplateController();

    void addSearchForm(GrpcSearchForm * searchForm);
    virtual void addActionBars(QMainWindow * mainWindow, QMenuBar * menuBar, QToolBar * toolBar, QStatusBar * statusBar);

    enum State {Unselected = 0, Browsing, Edit, Insert};

signals:
    void rowChanged(const QModelIndex & index);
    void populateModel(std::shared_ptr<IBaseDataContainer> container);
    void masterRowChanged(const QModelIndex & index);

    void showStatusMessage(const QString & message, int timeOut = 0);

    void startInsert();
    void startEdit();
    void finishSave();

    void hideOthers(GrpcTemplateController * controller);
    void focusIn();

public slots:
    virtual void masterChanged(const QModelIndex & index);
    void applySearchCriterias( const JsonParameterFormatter & searchCriterias);
    void showMenuAndToolbar();
    void hideMenuAndToolbar();

private slots:
    void currentChanged(const QModelIndex & current, const QModelIndex & previous);
    void formContentChanged();

    void refresh_all();
    void add_new_record();
    void edit_record();
    void delete_record();
    void save_record();
    void escape();

protected:
    // be sure to override it in the child
    virtual void modelData() = 0;

    virtual bool refreshGrpc() {return true;}
    virtual bool addNewGrpc() {return true;}
    virtual bool editGrpc() {return true;}
    virtual bool deleteGrpc() {return true;}

    virtual void updateState();

    QVariant masterVariantObject();
    JsonParameterFormatter & searchCriterias();

    QMenu * templateMenu(){return m_templateMenu;}
    QMenu * contextMenu(){return m_contextMenu;}
    QToolBar * templateToolBar(){return m_templateToolBar;}

private:
    void initActions();

    JsonParameterFormatter m_searchCriterias;
    std::unique_ptr<IBaseGrpcObjectWrapper> m_masterObjectWrapper;
    int m_currentRow = -1;
    State m_state = Unselected;

    QMenu * m_templateMenu = nullptr;
    QMenu * m_contextMenu = nullptr;
    QToolBar * m_templateToolBar = nullptr;

    QAction * m_actionRefresh;
    QAction * m_actionAddNew;
    QAction * m_actionEdit;
    QAction * m_actionDelete;
    QAction * m_actionSave;
    QAction * m_actionEscape;
};

#endif // GRPCTEMPLATECONTROLLER_H
