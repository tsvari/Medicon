#ifndef SQLCOMMAND_H
#define SQLCOMMAND_H

#include <SQLAPI.h>

#include "sqlapplet.h"

class SqlConnection;
class SqlCommand  : public SACommand
{
public:
    SqlCommand(SqlConnection & connection,
               const char * appletName,
               map<string, string> formattedParamValueList = {},
               const SAString& sCmd = SAString(),
               SACommandType_t eCmdType = SA_CmdUnknown);

    void addDataInfo(const char * paramName, const char * paramValue);
    void addDataInfo(const char * paramName, int paramValue);
    void addDataInfo(const char * paramName, double paramValue);
    void addDataInfo(const char * paramName, bool paramValue);
    void addDataInfo(const char * paramName, const std::chrono::sys_seconds paramValue, DataInfo::Type nType);
    void addDataInfo(const char * paramName,  const char * paramValue, DataInfo::Type nType);

    void execute();

protected:
    SQLApplet m_applet;
};

#endif // SQLCOMMAND_H
