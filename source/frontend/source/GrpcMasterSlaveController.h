#ifndef GRPCMASTERSLAVECONTROLLER_H
#define GRPCMASTERSLAVECONTROLLER_H

#include <QObject>

class GrpcTemplateController;
class GrpcMasterSlaveController : public QObject
{
    Q_OBJECT
public:
    explicit GrpcMasterSlaveController(GrpcTemplateController * master, GrpcTemplateController * slave, QObject *parent = nullptr);

    void addMasterSlave(GrpcTemplateController * master, GrpcTemplateController * slave);

    struct MasterSlave {
        MasterSlave(GrpcTemplateController * master, GrpcTemplateController * slave)
            : master(master)
            , slave(slave){}
        GrpcTemplateController * master = nullptr;
        GrpcTemplateController * slave = nullptr;
    };

signals:

private:
    QList<MasterSlave> m_masterSlaveList;

};

#endif // GRPCMASTERSLAVECONTROLLER_H
