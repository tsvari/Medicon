#ifndef GRPCFORM_H
#define GRPCFORM_H

#include <QWidget>
#include <QTabBar>
#include <QTabWidget>

#include "GrpcObjectWrapper.hpp"

class GrpcForm : public QWidget
{
    Q_OBJECT
public:
    explicit GrpcForm(IBaseGrpcObjectWrapper * objectWrapper, QWidget * parent = nullptr);
    virtual ~GrpcForm();

    QVariant object();
    QTabWidget * tabWidget();

public slots:
    // Can be overridden in a child class if needed
    virtual void fill(const QModelIndex & index);
    virtual void clear();
    virtual void fillObject();
    void selectTab();

protected:
    friend class GrpcTemplateController;
    // be sure to override it in the child class
    virtual void initializeForm() = 0;
    IBaseGrpcObjectWrapper * objectWrapper() {return m_objectWrapper.get();}

    void focusInEvent(QFocusEvent * event) override;
    void focusOutEvent(QFocusEvent * event) override;

signals:
    void contentChanged();
    void focusIn();
    void focusOut();

private:
    void fillWidget(QWidget * widget, const DataInfo::Type & type, const QVariant & data);
    QVariant widgetData(QWidget * widget, const DataInfo::Type & type);
    void initilizeWidgets();
    QTabBar * tabBar();
    int tabIndex();

    QList<QWidget*> m_formWidgets;
    std::unique_ptr<IBaseGrpcObjectWrapper> m_objectWrapper = nullptr;
    QIcon m_saveIcon;
};

#endif // GRPCFORM_H
