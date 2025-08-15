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
    // purely virtual, be sure to override it in the child
    virtual void initializeData() = 0;

    QVariant object();

public slots:
    virtual void initializeWrapper(const QVariant & varData) = 0;

protected:
    void initialize();

    void fillForm();
    void fillObject();

    IBaseGrpcObjectWrapper * m_objectWrapper = nullptr;

private:
    void fillWidget(QWidget * widget, const DataInfo::Type & type, const QVariant & data);
    QList<QWidget*> m_formWidgets;
};

#endif // GRPCFORM_H
