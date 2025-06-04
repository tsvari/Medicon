#include "sqlapplet.h"
#include "Markup.h"
#include <format>
#include <filesystem>

namespace {
    static string AppletPath = "";
    static bool UseDefaultValue = false;
    map<string, DataInfo::Type> const xmlTypeToDataInfo={{"NUMERIC", DataInfo::Int},
                                      {"STRING", DataInfo::String},
                                      {"DATETIME", DataInfo::DateTime},
                                      {"DATE", DataInfo::Date},
                                      {"TIME", DataInfo::Time}};
}
namespace fs = std::filesystem;
SQLApplet::SQLApplet(const char * appletName, map<string, string> formattedParamValueList):
    m_paramValueList(formattedParamValueList)
{
    // Don't forget call SQLApplet::InitPathToApplets to initialize global path
    if(AppletPath.size() == 0) {
        throw SQLAppletException(APPLET_ERR_INIT);
    }
    m_appletPath = std::format("{}{}", AppletPath, appletName);
}

SQLApplet::SQLApplet(const char *appletName, const string & parametersXml)
{
    // Don't forget call SQLApplet::InitPathToApplets to initialize global path
    if(AppletPath.size() == 0) {
        throw SQLAppletException(APPLET_ERR_INIT);
    }
    m_appletPath = std::format("{}{}", AppletPath, appletName);
    m_paramValueList = JsonParameterFormatter::fromJsonString(parametersXml);
}

void SQLApplet::InitPathToApplets(const char * appletPath, bool useDefaultValue)
{
    AppletPath = appletPath;
    UseDefaultValue = useDefaultValue;
}

void SQLApplet::AddDataInfo(const char * paramName, const char * paramValue)
{
    FormatterDataType data(paramValue);
    m_formatter.AddDataInfo(paramName, data);
}

void SQLApplet::AddDataInfo(const char * paramName, int paramValue)
{
    FormatterDataType data(paramValue);
    m_formatter.AddDataInfo(paramName, data);
}

void SQLApplet::AddDataInfo(const char *paramName, int64_t paramValue)
{
    FormatterDataType data(paramValue);
    m_formatter.AddDataInfo(paramName, data);
}

void SQLApplet::AddDataInfo(const char * paramName, double paramValue)
{
    FormatterDataType data(paramValue);
    m_formatter.AddDataInfo(paramName, data);
}

void SQLApplet::AddDataInfo(const char * paramName, bool paramValue)
{
    FormatterDataType data(paramValue);
    m_formatter.AddDataInfo(paramName, data);
}

void SQLApplet::AddDataInfo(const char * paramName, const std::chrono::sys_seconds paramValue, DataInfo::Type nType)
{
    m_formatter.AddDataInfo(paramName, paramValue, nType);
}

void SQLApplet::AddDataInfo(const char *paramName, const char *paramValue, DataInfo::Type nType)
{
    m_formatter.AddDataInfo(paramName, paramValue, nType);
}

void SQLApplet::parse()
{
    // check existense of applett and data content
    if(!fs::exists(m_appletPath)) {
        throw SQLAppletException(APPLET_ERR_WRONG_PATH);
    }

    map<string, string> formattedList(m_formatter.formattedParamValueList());
    formattedList.insert(m_paramValueList.begin(), m_paramValueList.end());

    CMarkup	parser;
    // reuse parser
    if (!parser.Load(m_appletPath)) {
        throw SQLAppletException(string(string(APPLET_ERR_PARAM_XML) + string(" - ") + parser.GetError()).c_str());
    }

    // find Description Tag/Value
    if(!parser.FindChildElem("Description")) {
        throw SQLAppletException(APPLET_ERR_NO_DESCRIPTION);
    }

    m_description = parser.GetChildData();
     vector<DataInfo> xmlDataInfoParams;
    // - find params and fill vector
    while (parser.FindChildElem("Param")) {
        parser.IntoElem();

        DataInfo ob;

        if (parser.FindChildElem("Name")) {
            ob.param = parser.GetChildData();
        } else {
            throw SQLAppletException(APPLET_ERR_PARAM_NO_NAME);
        }
        if (parser.FindChildElem("Type")) {
            string sType = parser.GetChildData();
            ob.type = xmlTypeToDataInfo.at(sType);
        } else {
            throw SQLAppletException(APPLET_ERR_PARAM_NO_TYPE);
        }

        // check param/value in formattedList
        if(formattedList.find(ob.param) == formattedList.end()) {
            if(UseDefaultValue) {
                if (parser.FindChildElem("DefVal")) {
                    string defaultValue = parser.GetChildData();
                    if(defaultValue == "uuid") {
                        ob.value = TimeFormatHelper::generateUniqueString();
                    } else {
                        ob.value = parser.GetChildData();
                    }
                } else {
                    throw SQLAppletException(APPLET_ERR_PARAM_NO_DEFVAL);
                }
            }
        } else {
            if(formattedList.contains(ob.param)) {
                ob.value = formattedList[ob.param];
            } else {
                throw SQLAppletException(APPLET_ERR_PARAM_NO_VALUE);
            }
        }

        xmlDataInfoParams.push_back(ob);
        parser.OutOfElem();
    }

    // - find SQL code
    if (!parser.FindChildElem("Code")) {
        throw SQLAppletException(APPLET_ERR_PARAM_NO_CODE);
    }

    m_sqlSource = parser.GetChildData();

    // replace params by value
    for( DataInfo & ob: xmlDataInfoParams) {
        string sTo = ":" + ob.param + ":";

        if(ob.type == DataInfo::String ) {
            ob.value = "'" + ob.value + "'";
        } else if(ob.type == DataInfo::DateTime ||
                   ob.type == DataInfo::Date ||
                   ob.type == DataInfo::Time) {
            if(	ob.value != string("NULL") ) {
                ob.value = "'" + ob.value + "'";
            } else {
                ob.value = "NULL";
            }
        }

        size_t start_pos = m_sqlSource.find(sTo);
        while(start_pos && start_pos < m_sqlSource.length()) {
            m_sqlSource.replace(start_pos, sTo.length(), ob.value);
            start_pos = m_sqlSource.find(sTo, start_pos);
        }
    }
    m_sqlSource = Trimmer::trim(m_sqlSource);
}
