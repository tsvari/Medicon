#ifndef SQLQUERY_H
#define SQLQUERY_H

#include "sqlcommand.h"

class SqlConnection;
class SqlQuery : public SqlCommand
{
public:
    SqlQuery(SqlConnection & connection,
             const char * appletName,
             map<string, string> formattedParamValueList = {},
             const SAString& sCmd = SAString(),
             SACommandType_t eCmdType = SA_CmdUnknown);
    bool query();
};

#endif // SQLQUERY_H
