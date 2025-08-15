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
    virtual void initializeWrapper(const QModelIndex & index);

protected:
    // be sure to override it in the child class
    virtual void initializeData();

    // Can be overridden in a child class if needed
    virtual void fillForm();
    virtual void fillObject();

    IBaseGrpcObjectWrapper * m_objectWrapper = nullptr;

private:
    friend class GrpcUiTemplate;

    void fillWidget(QWidget * widget, const DataInfo::Type & type, const QVariant & data);
    QList<QWidget*> m_formWidgets;
};

#endif // GRPCFORM_H
