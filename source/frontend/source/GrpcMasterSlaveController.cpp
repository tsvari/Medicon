#include "GrpcMasterSlaveController.h"
#include "GrpcTemplateController.h"

GrpcMasterSlaveController::GrpcMasterSlaveController(GrpcTemplateController * master, GrpcTemplateController * slave, QObject *parent)
    : QObject{parent}
{
    addMasterSlave(master, slave);
}

void GrpcMasterSlaveController::addMasterSlave(GrpcTemplateController * master, GrpcTemplateController * slave)
{
    connect(master, &GrpcTemplateController::rowChanged, slave, &GrpcTemplateController::masterRowChanged);
    m_masterSlaveList.push_back({master, slave});
}
