#include "sqlapplet.h"
#include "Markup.h"
#include <format>
#include <filesystem>
#include <cassert>

using std::map;

namespace fs = std::filesystem;
namespace {
    static string AppletPath = "";
    map<string, DataInfo::Type> const xmlTypeToDataInfo={{"NUMERIC", DataInfo::Int},
                                      {"STRING", DataInfo::String},
                                      {"DATETIME", DataInfo::DateTime},
                                      {"DATE", DataInfo::Date},
                                      {"TIME", DataInfo::Time}};
}

SQLApplet::SQLApplet(const char * appletName, map<string, string> formattedParamValueList):
    m_formattedParamValueList(formattedParamValueList)
{
    // If you forget call SQLApplet::InitPathToApplets to initialize global path
    assert(AppletPath.size() > 0);
    m_appletPath = std::format("{}{}", AppletPath, appletName);
}

SQLApplet::SQLApplet(const char *appletName, const TypeToStringFormatter & formatter) :
    m_formattedParamValueList(formatter.formattedParamValueList())
{
    // If you forget call SQLApplet::InitPathToApplets to initialize global path
    assert(AppletPath.size() > 0);
    m_appletPath = std::format("{}{}", AppletPath, appletName);
}

void SQLApplet::InitPathToApplets(const char * appletPath)
{
    AppletPath = appletPath;
}

void SQLApplet::parse()
{
    // check existense of applett and data content
    if(!fs::exists(m_appletPath)) {
        throw SQLAppletException(APPLET_ERR_WRONG_PATH.c_str());
    }

    CMarkup	parser;
    // reuse parser
    if (!parser.Load(m_appletPath)) {
        throw SQLAppletException(parser.GetError().c_str() );
    }

    // find Description Tag/Value
    if(!parser.FindChildElem("Description")) {
        throw SQLAppletException(APPLET_ERR_NO_DESCRIPTION.c_str());
    }

    description_ = parser.GetChildData();
     vector<DataInfo> xmlDataInfoParams;
    // - find params and fill vector
    while (parser.FindChildElem("Param")) {
        parser.IntoElem();

        DataInfo ob;

        if (parser.FindChildElem("Name")) {
            ob.param = parser.GetChildData();
        } else {
            throw SQLAppletException(APPLET_ERR_PARAM_NO_NAME.c_str());
        }
        if (parser.FindChildElem("Type")) {
            string sType = parser.GetChildData();
            ob.type = xmlTypeToDataInfo.at(sType);
        } else {
            throw SQLAppletException(APPLET_ERR_PARAM_NO_TYPE.c_str());
        }

        // check param/value in m_formattedParamValueList
        if(m_formattedParamValueList.find(ob.param) == m_formattedParamValueList.end()) {
            if (parser.FindChildElem("DefVal")) {
                ob.value = parser.GetChildData();
            } else {
                throw SQLAppletException(APPLET_ERR_PARAM_NO_DEFVAL.c_str());
            }
        } else {
            if(m_formattedParamValueList.contains(ob.param)) {
                ob.value = m_formattedParamValueList[ob.param];
            } else {
                throw SQLAppletException(APPLET_ERR_PARAM_NO_VALUE.c_str());
            }
        }

        xmlDataInfoParams.push_back(ob);
        parser.OutOfElem();
    }

    // - find SQL code
    if (!parser.FindChildElem("Code")) {
        throw SQLAppletException(APPLET_ERR_PARAM_NO_CODE.c_str());
    }

    m_sqlSource = parser.GetChildData();

    // replace params by value
    for( DataInfo & ob: xmlDataInfoParams) {
        string sTo = ":" + ob.param;

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
    m_sqlSource = trimLeftRight(m_sqlSource);
}
