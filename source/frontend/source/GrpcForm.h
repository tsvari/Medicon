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

protected:
    friend class GrpcTemplateController;
    // be sure to override it in the child class
    virtual void initializeForm() = 0;
    // Can be overridden in a child class if needed
    virtual void fillObject();
    void initilizeWidget();

    IBaseGrpcObjectWrapper * objectWrapper() {return m_objectWrapper;}

private:
    void fillWidget(QWidget * widget, const DataInfo::Type & type, const QVariant & data);
    QList<QWidget*> m_formWidgets;

    IBaseGrpcObjectWrapper * m_objectWrapper = nullptr;
};

#endif // GRPCFORM_H
