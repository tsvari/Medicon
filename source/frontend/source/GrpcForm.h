#ifndef GRPCFORM_H
#define GRPCFORM_H

#include <QWidget>

#include "GrpcObjectWrapper.hpp"

class GrpcForm : public QWidget
{
    Q_OBJECT
public:
    explicit GrpcForm(IBaseGrpcObjectWrapper * objectWrapper, QWidget *parent = nullptr);
    virtual ~GrpcForm();

    QVariant object();

public slots:
    // Can be overridden in a child class if needed
    virtual void fillForm(const QModelIndex & index);
    virtual void clearForm();
    virtual void fillObject();

protected:
    friend class GrpcTemplateController;
    // be sure to override it in the child class
    virtual void initializeForm() = 0;
    void initilizeWidget();

    IBaseGrpcObjectWrapper * objectWrapper() {return m_objectWrapper.get();}

private:
    void fillWidget(QWidget * widget, const DataInfo::Type & type, const QVariant & data);
    QVariant widgetData(QWidget * widget, const DataInfo::Type & type);

    QList<QWidget*> m_formWidgets;
    std::unique_ptr<IBaseGrpcObjectWrapper> m_objectWrapper = nullptr;
};

#endif // GRPCFORM_H
