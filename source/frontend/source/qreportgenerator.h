#pragma once

#include <string>
#include <vector>
#include <map>

#include "global_def.h"

using namespace std;

typedef map<std::string, std::string>  XMLRepRow;

typedef struct _PARAM_OBJECT_REP
    {
    string		m_sName;
    string		m_sValue;
    unsigned int m_type;
    unsigned int m_nPrecision;
    }PARAM_OBJECT_REP;

class QReportGenerator
{
public:
    QReportGenerator();
    QReportGenerator(const string& sHtmlContent);
    ~QReportGenerator();

    //enum VariableType { Int = 0, string = 1, Double = 2, DateTime = 3, DateTimeNoSec = 4, Date = 5, Time = 6 };

    void AddBodyVariable(const char* sBodyVarName, const std::string&  sBodyVarValue);
    void AddBodyVariable(const char* sBodyVarName, int    nBodyVarValue);
    void AddBodyVariable(const char* sBodyVarName, double dBodyVarValue, short int nPrecision);
    void AddBodyVariable(const char* sBodyVarName, time_t dBodyVarValue, unsigned int nType);

    void AddNewRow();

    void AddRowVariable(const char* sRowVarName, const std::string& sRowVarValue);
    void AddRowVariable(const char* sRowVarName, int    nRowVarValue);
    void AddRowVariable(const char* sRowVarName, double dRowVarValue, short int nPrecision);
    void AddRowVariable(const char* sRowVarName, time_t dRowVarValue, unsigned int nType);

    void SetHtmlContent(const std::string& sHtmlContent){_htmlContent = sHtmlContent;}
    std::string GetHtmlContent();

    void SetRostring(const std::string& sRostring){_rowString = sRostring;}
protected:
    vector<PARAM_OBJECT_REP> _varBody;
    vector<XMLRepRow> _varRow;
    std::string _htmlContent;
    int	_nCurrentRow;
    std::string _rowString;
};

