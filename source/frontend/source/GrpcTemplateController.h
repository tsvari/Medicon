#ifndef GRPCTEMPLATECONTROLLER_H
#define GRPCTEMPLATECONTROLLER_H

#include <QObject>

class GrpcUiTemplate;
class GrpcTemplateController : public QObject
{
    Q_OBJECT
public:
    explicit GrpcTemplateController(GrpcUiTemplate * master, GrpcUiTemplate * slave, QObject *parent = nullptr);

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

#endif // GRPCTEMPLATECONTROLLER_H
