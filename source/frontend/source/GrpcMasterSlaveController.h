#ifndef GRPCMASTERSLAVECONTROLLER_H
#define GRPCMASTERSLAVECONTROLLER_H

#include <QObject>

class GrpcUiTemplate;
class GrpcMasterSlaveController : public QObject
{
    Q_OBJECT
public:
    explicit GrpcMasterSlaveController(GrpcUiTemplate * master, GrpcUiTemplate * slave, QObject *parent = nullptr);

    void addMasterSlave(GrpcUiTemplate * master, GrpcUiTemplate * slave);

    struct MasterSlave {
        MasterSlave(GrpcUiTemplate * master, GrpcUiTemplate * slave)
            : master(master)
            , slave(slave){}
        GrpcUiTemplate * master = nullptr;
        GrpcUiTemplate * slave = nullptr;
    };

signals:

private:
    QList<MasterSlave> m_masterSlaveList;

};

#endif // GRPCMASTERSLAVECONTROLLER_H
