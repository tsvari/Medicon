#include "GrpcMasterSlaveController.h"
#include "GrpcTemplateController.h"

#include <QSet>

GrpcMasterSlaveController::GrpcMasterSlaveController(GrpcTemplateController * master, GrpcTemplateController * slave, QObject *parent)
    : QObject{parent}
{
    addMasterSlave(master, slave);
}

void GrpcMasterSlaveController::addMasterSlave(GrpcTemplateController * master, GrpcTemplateController * slave)
{
    connect(master, &GrpcTemplateController::rowChanged, slave, &GrpcTemplateController::masterRowChanged);

    // Clear slave data as soon as the master begins reloading.
    connect(master, &GrpcTemplateController::loadingStarted, slave, &GrpcTemplateController::clearForMasterReload);

    connect(master, &GrpcTemplateController::hideOthers, this, &GrpcMasterSlaveController::hideAllMenuToolbars);
    connect(slave, &GrpcTemplateController::hideOthers, this, &GrpcMasterSlaveController::hideAllMenuToolbars);

    m_masterSlaveList.push_back({master, slave});
}

void GrpcMasterSlaveController::clearAll()
{
    QSet<GrpcTemplateController *> uniqueMasters;
    for (const auto & masterSlave : std::as_const(m_masterSlaveList)) {
        if (masterSlave.master) {
            uniqueMasters.insert(masterSlave.master);
        }
    }

    for (auto * master : uniqueMasters) {
        master->clearModel();
    }
}

void GrpcMasterSlaveController::hideAllMenuToolbars(GrpcTemplateController * controller)
{
    for(auto const & masterSlave: std::as_const(m_masterSlaveList)) {
        if(masterSlave.master != controller) {
            masterSlave.master->hideMenuAndToolbar();
        }
        if(masterSlave.slave != controller) {
            masterSlave.slave->hideMenuAndToolbar();
        }
    }
}
