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
class GrpcTemplateController : public QObject
{
    Q_OBJECT
public:
    explicit GrpcTemplateController(GrpcProxySortFilterModel * proxyModel, QAbstractItemView  * tableView, GrpcForm * form, IBaseGrpcObjectWrapper * masterObjectWrapper, QObject *parent = nullptr);
    virtual ~GrpcTemplateController();

    void addSearchForm(GrpcSearchForm * searchForm);
    void addActionBars(QMainWindow * mainWindow, QMenuBar * menuBar, QToolBar * toolBar, QStatusBar * statusBar);

    enum State {Unselected = 0, Browsing, Edit, Insert};

signals:
    void rowChanged(const QModelIndex & index);
    void populateModel(std::shared_ptr<IBaseDataContainer> container);
    void masterRowChanged(const QModelIndex & index);
    void clearForm();

    void showStatusMessage(const QString & message, int timeOut = 0);

public slots:
    virtual void masterChanged(const QModelIndex & index);
    void applySearchCriterias( const JsonParameterFormatter & searchCriterias);

private slots:
    void currentChanged(const QModelIndex & current, const QModelIndex & previous);
    void updateState();

    void refresh_all();
    void add_new_record();
    void edit_record();
    void delete_record();
    void save_record();

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

    QMenuBar * m_mainMenuBar = nullptr;
    QToolBar * m_mainToolBar = nullptr;

    QMenu * m_templateMenu = nullptr;
    QToolBar * m_templateToolBar = nullptr;

    QAction * m_actionRefresh;
    QAction * m_actionAddNew;
    QAction * m_actionEdit;
    QAction * m_actionDelete;
    QAction * m_actionSave;
};

#endif // GRPCTEMPLATECONTROLLER_H
