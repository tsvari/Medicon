#include "GrpcMasterSlaveController.h"
#include "GrpcUiTemplate.h"

GrpcMasterSlaveController::GrpcMasterSlaveController(GrpcUiTemplate * master, GrpcUiTemplate * slave, QObject *parent)
    : QObject{parent}
{
    addMasterSlave(master, slave);
}

void GrpcMasterSlaveController::addMasterSlave(GrpcUiTemplate * master, GrpcUiTemplate * slave)
{
    connect(master, &GrpcUiTemplate::rowChanged, slave, &GrpcUiTemplate::masterRowChanged);
    m_masterSlaveList.push_back({master, slave});
}
