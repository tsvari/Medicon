#include "sqlquery.h"

SqlQuery::SqlQuery(SqlConnection & connection,
                   const char * appletName,
                   map<string, string> formattedParamValueList,
                   const SAString & sCmd,
                   SACommandType_t eCmdType) :
    SqlCommand(connection, appletName, formattedParamValueList, sCmd, eCmdType)
{}

bool SqlQuery::query()
{
    if(!m_executed) {
        execute();
        return FetchNext() & isResultSet();
    } else {
        return FetchNext() ;
    }
}
