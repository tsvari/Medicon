#include <sstream>
#include <iostream>
#include <ctime>
#include <fstream>
#include "qreportgenerator.h"
//#include	"easylogging++.h"
//#include <QMessageBox>

QReportGenerator::QReportGenerator():
    _htmlContent(""),
    _nCurrentRow (-1),
    _rowString ("row")
{

}

QReportGenerator::QReportGenerator(const std::string& sHtmlContent):
    _nCurrentRow (-1),
    _rowString ("row")
{
    SetHtmlContent(sHtmlContent);
}

QReportGenerator::~QReportGenerator()
{

}

namespace qPatch
{
    template < typename T > std::string to_string(const T& n)
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}


//////////////////////////////////////////////////////////////////////////////////
/// \brief QReportGenerator::AddBodyVariable
/// \param sBodyVarName
/// \param sBodyVarValue
///
void QReportGenerator::AddBodyVariable(const char* sBodyVarName, const std::string& sBodyVarValue)
{
    PARAM_OBJECT_REP info;
    info.m_type = GlobalType::Int;
    info.m_sName = sBodyVarName;
    info.m_sValue = sBodyVarValue;
    info.m_nPrecision = 0;

    _varBody.push_back(info);
}

//////////////////////////////////////////////////////////////////////////////////
/// \brief QReportGenerator::AddBodyVariable
/// \param sBodyVarName
/// \param nBodyVarValue
///
void QReportGenerator::AddBodyVariable(const char* sBodyVarName, int    nBodyVarValue)
{
    PARAM_OBJECT_REP info;
    info.m_type = GlobalType::String;
    info.m_sName = sBodyVarName;
    info.m_sValue = qPatch::to_string(nBodyVarValue);
    info.m_nPrecision = 0;

     _varBody.push_back(info);
}

//////////////////////////////////////////////////////////////////////////////////
/// \brief QReportGenerator::AddBodyVariable
/// \param sBodyVarName
/// \param dBodyVarValue
/// \param nDecimal
///
void QReportGenerator::AddBodyVariable(const char* sBodyVarName, double dBodyVarValue, short int nPrecision)
{
    PARAM_OBJECT_REP info;
    info.m_type = GlobalType::Double;
    info.m_sName = sBodyVarName;
    info.m_nPrecision = nPrecision;

    char tmp[24], tmpFrmt[6];

#if defined(_WIN32) && defined(_MSC_VER)
    sprintf_s(tmpFrmt, "%s%df", "%.", nPrecision);
    sprintf_s(tmp, tmpFrmt,  dBodyVarValue);
#else
    snprintf(tmpFrmt, 6, "%s%df", "%.", nPrecision);
    snprintf(tmp, 24, tmpFrmt,  dBodyVarValue);
#endif

    std::string s(tmp);
	info.m_sValue.assign(s.begin(), s.end());

     _varBody.push_back(info);
}

//////////////////////////////////////////////////////////////////////////////////
/// \brief QReportGenerator::AddBodyVariable
/// \param sBodyVarName
/// \param dBodyVarValue
/// \param nDateType
///
void QReportGenerator::AddBodyVariable(const char* sBodyVarName, time_t dBodyVarValue, unsigned int nType)
{
    PARAM_OBJECT_REP info;
    info.m_type = nType;
    info.m_nPrecision = 0;
    info.m_sName = sBodyVarName;

    time_t dVal = dBodyVarValue;

	if(dVal<=0)
		{
        info.m_sValue = "";
		_varBody.push_back(info);
		return;
		}

    struct tm *ptm2=NULL;

    #if defined(_WIN32) && defined(_MSC_VER)
        struct tm ptm;
        localtime_s(&ptm, &dVal);
        ptm2 = &ptm;
    #else
        ptm2 = localtime(&dVal);
    #endif

	char buffer[32];

    switch(nType)
        {
        case GlobalType::DateTime:
            strftime (buffer, 32, "%m-%d-%Y %H:%M:%S", ptm2);
            break;
        case GlobalType::DateTimeNoSec:
            strftime (buffer, 32, "%m-%d-%Y %H:%M", ptm2);
            break;
        case GlobalType::Date:
            strftime (buffer, 32, "%m-%d-%Y", ptm2);
            break;
        case GlobalType::Time:
            strftime (buffer, 32, "%H:%M:%S", ptm2);
            break;
        }
	std::string s(buffer);
	info.m_sValue.assign(s.begin(), s.end());

    _varBody.push_back(info);
}

//////////////////////////////////////////////////////////////////////////////////
/// \brief QReportGenerator::AddRowVariable
/// \param sRowVarName
/// \param sRowVarValue
///
void QReportGenerator::AddRowVariable(const char* sRowVarName, const std::string& sRowVarValue)
{
    if(_nCurrentRow<0)
        return;

    _varRow[_nCurrentRow][sRowVarName] = sRowVarValue;
}

//////////////////////////////////////////////////////////////////////////////////
/// \brief QReportGenerator::AddRowVariable
/// \param sRowVarName
/// \param nRowVarValue
///
void QReportGenerator::AddRowVariable(const char* sRowVarName, int    nRowVarValue)
{
    if(_nCurrentRow<0)
        return;

     _varRow[_nCurrentRow][sRowVarName] = qPatch::to_string(nRowVarValue);
}

//////////////////////////////////////////////////////////////////////////////////
/// \brief QReportGenerator::AddRowVariable
/// \param sRowVarName
/// \param dRowVarValue
/// \param nDecimal
///
void QReportGenerator::AddRowVariable(const char* sRowVarName, double dRowVarValue, short int nPrecision)
{
    if(_nCurrentRow<0)
        return;

    char tmp[24], tmpFrmt[6];

#if defined(_WIN32) && defined(_MSC_VER)
    sprintf_s(tmpFrmt, "%s%df", "%.", nPrecision);
    sprintf_s(tmp, tmpFrmt,  dRowVarValue);
#else
    snprintf(tmpFrmt, 6, "%s%df", "%.", nPrecision);
    snprintf(tmp, 24, tmpFrmt,  dRowVarValue);
#endif

	std::string s(tmp);
    _varRow[_nCurrentRow][sRowVarName].assign(s.begin(), s.end());
}

//////////////////////////////////////////////////////////////////////////////////
/// \brief QReportGenerator::AddRowVariable
/// \param sRowVarName
/// \param dRowVarValue
/// \param nDateType
///
void QReportGenerator::AddRowVariable(const char* sRowVarName, time_t dRowVarValue, unsigned int nType)
{
    if(_nCurrentRow<0)
        return;

    time_t dVal = dRowVarValue;

	if(dVal<=0)
		{
        _varRow[_nCurrentRow][sRowVarName] = "";
		return;
		}

    struct tm *ptm2=NULL;

    #if defined(_WIN32) && defined(_MSC_VER)
        struct tm ptm;
        localtime_s(&ptm, &dVal);
        ptm2 = &ptm;
    #else
        ptm2 = localtime(&dVal);
    #endif

	char buffer[32];

    switch(nType)
        {
    case GlobalType::DateTime:
        strftime (buffer, 32, "%m-%d-%Y %H:%M:%S", ptm2);
        break;
    case GlobalType::DateTimeNoSec:
        strftime (buffer, 32, "%m-%d-%Y %H:%M", ptm2);
        break;
    case GlobalType::Date:
        strftime (buffer, 32, "%m-%d-%Y", ptm2);
        break;
    case GlobalType::Time:
        strftime (buffer, 32, "%H:%M:%S", ptm2);
        break;
        }

	std::string s(buffer);
    _varRow[_nCurrentRow][sRowVarName].assign(s.begin(), s.end());
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief std::string::AddNewRow
///
void QReportGenerator::AddNewRow()
{
    XMLRepRow pNewRow;
    _varRow.push_back( pNewRow );
    _nCurrentRow++;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief QReportGenerator::GetHtmlContent
/// \return
///
std::string QReportGenerator::GetHtmlContent()
{
    std::string sGenerated="Problems with Template( <row> or </row>";

    std::string sHtml=_htmlContent;

    std::string sRowStart = "<" + _rowString + ">";
    std::string sRowEnd = "</" + _rowString + ">";

    size_t nStart = sHtml.find(sRowStart, 0);
    size_t nEnd = sHtml.find(sRowEnd, 0);

    if (nStart != std::string::npos && nEnd != std::string::npos)
        {
        size_t nSubStart = nStart +_rowString.size()+2;

        std::string sRowHtmlTmp = sHtml.substr(nSubStart, nEnd-nSubStart);

        std::string sLeft = sHtml.substr(0, nStart);

        std::string sRight = sHtml.substr(nEnd+sRowEnd.size(), sHtml.size() - nEnd+sRowEnd.size());

        std::string sHtmlRowOutput="";

        vector<XMLRepRow>::iterator it;
        for (it = _varRow.begin(); it != _varRow.end(); ++it)
            {
            XMLRepRow& row = *it;
            std::string sTmp = sRowHtmlTmp;
            std::map<std::string, std::string>::iterator iter;
            for (iter = row.begin(); iter != row.end(); ++iter)
                {

                std::string sName = iter->first;
                std::string sValue = iter->second;

                size_t start_pos = sTmp.find(sName);
                if (start_pos != std::string::npos )
                    sTmp.replace(start_pos, sName.length(), sValue);
                }
            sHtmlRowOutput += sTmp;

            }
        sGenerated = sLeft+sHtmlRowOutput+sRight;

        vector<PARAM_OBJECT_REP>::iterator itBody;
        for(itBody = _varBody.begin(); itBody != _varBody.end(); ++ itBody)
            {
            PARAM_OBJECT_REP& ob = (*itBody);
            size_t start_pos = sGenerated.find(ob.m_sName);
            if (start_pos != std::string::npos )
                sGenerated.replace(start_pos, ob.m_sName.length(), ob.m_sValue);
            }
        }

    //ofstream myfile;
    //myfile.open ("C:\\1.htm");
    //myfile << sGenerated;
    //myfile.close();


    return sGenerated;
}
