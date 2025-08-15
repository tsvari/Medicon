#include "GrpcTemplateController.h"
#include "GrpcUiTemplate.h"

GrpcTemplateController::GrpcTemplateController(GrpcUiTemplate * master, GrpcUiTemplate * slave, QObject *parent)
    : QObject{parent}
{
    addMasterSlave(master, slave);
}

void GrpcTemplateController::addMasterSlave(GrpcUiTemplate * master, GrpcUiTemplate * slave)
{
    connect(master, &GrpcUiTemplate::rowChanged, slave, &GrpcUiTemplate::masterRowChanged);
    m_masterSlaveList.push_back({master, slave});
}
