#include "GrpcSearchForm.h"

GrpcSearchForm::GrpcSearchForm(QWidget *parent)
    : QWidget{parent}
{}

void GrpcSearchForm::submit()
{
    emit startSearch({});
}
