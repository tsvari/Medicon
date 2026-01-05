#include "sqlapplet.h"
#include "Markup/Markup.h"
#include <format>
#include <filesystem>

using std::string;
using std::map;

namespace {
    static string AppletPath;
    static bool UseDefaultValue = false;
    
    // Type mapping table
    const map<string, DataInfo::Type> xmlTypeToDataInfo = {
        {"FIELD", DataInfo::Zero},
        {"NUMERIC", DataInfo::Int},
        {"STRING", DataInfo::String},
        {"DATETIME", DataInfo::DateTime},
        {"DATE", DataInfo::Date},
        {"TIME", DataInfo::Time}
    };
}

namespace fs = std::filesystem;

SQLApplet::SQLApplet(std::string_view appletName, map<string, string> formattedParamValueList)
    : m_paramValueList(std::move(formattedParamValueList))
{
    if (AppletPath.empty()) {
        throw SQLAppletException(APPLET_ERR_INIT);
    }
    m_appletPath = AppletPath + string(appletName);
}

SQLApplet::SQLApplet(std::string_view appletName, const string& parametersXml)
{
    if (AppletPath.empty()) {
        throw SQLAppletException(APPLET_ERR_INIT);
    }
    m_appletPath = AppletPath + string(appletName);
    m_paramValueList = m_formatter.fromJsonString(parametersXml);
}

void SQLApplet::InitPathToApplets(std::string_view appletPath, bool useDefaultValue)
{
    AppletPath = string(appletPath);
    UseDefaultValue = useDefaultValue;
}

// Template addParameter is now defined in the header

void SQLApplet::addParameter(std::string_view name, const std::chrono::milliseconds paramValue, DataInfo::Type nType)
{
    m_formatter.addParameter(name, paramValue, nType);
}

void SQLApplet::addParameter(std::string_view name, const char *paramValue, DataInfo::Type nType)
{
    m_formatter.addParameter(name, paramValue, nType);
}

void SQLApplet::parse()
{
    // Check if file exists
    if (!fs::exists(m_appletPath)) {
        throw SQLAppletException(string(APPLET_ERR_WRONG_PATH) + m_appletPath);
    }

    // Merge formatter parameters with initial param list
    map<string, string> formattedList = m_formatter.toMap();
    formattedList.insert(m_paramValueList.begin(), m_paramValueList.end());

    // Load and parse XML
    CMarkup parser;
    if (!parser.Load(m_appletPath)) {
        throw SQLAppletException(std::format("{}: {}", APPLET_ERR_PARAM_XML, parser.GetError()));
    }

    // Get description
    if (!parser.FindChildElem("Description")) {
        throw SQLAppletException(APPLET_ERR_NO_DESCRIPTION);
    }
    m_description = parser.GetChildData();

    // Parse parameters
    std::vector<DataInfo> xmlDataInfoParams;
    while (parser.FindChildElem("Param")) {
        parser.IntoElem();
        DataInfo ob;

        // Get parameter name
        if (!parser.FindChildElem("Name")) {
            throw SQLAppletException(APPLET_ERR_PARAM_NO_NAME);
        }
        ob.param = parser.GetChildData();

        // Get parameter type
        if (!parser.FindChildElem("Type")) {
            throw SQLAppletException(std::format("{} for parameter '{}'", APPLET_ERR_PARAM_NO_TYPE, ob.param));
        }
        string sType = parser.GetChildData();
        auto typeIt = xmlTypeToDataInfo.find(sType);
        if (typeIt == xmlTypeToDataInfo.end()) {
            throw SQLAppletException(std::format("{} Type: '{}'", APPLET_ERR_WRONG_TYPE_NAME, sType));
        }
        ob.type = typeIt->second;

        // Get parameter value
        auto valueIt = formattedList.find(ob.param);
        if (valueIt == formattedList.end()) {
            // No value provided
            if (UseDefaultValue) {
                if (!parser.FindChildElem("DefVal")) {
                    throw SQLAppletException(std::format("{} for parameter '{}'", APPLET_ERR_PARAM_NO_DEFVAL, ob.param));
                }
                string defaultValue = parser.GetChildData();
                ob.value = (defaultValue == "uuid") ? TimeFormatHelper::generateUniqueString() : defaultValue;
            } else {
                throw SQLAppletException(std::format("{}: '{}'", APPLET_ERR_PARAM_NO_VALUE, ob.param));
            }
        } else {
            ob.value = valueIt->second;
        }

        xmlDataInfoParams.push_back(std::move(ob));
        parser.OutOfElem();
    }

    // Get SQL code
    if (!parser.FindChildElem("Code")) {
        throw SQLAppletException(APPLET_ERR_PARAM_NO_CODE);
    }
    m_sqlSource = parser.GetChildData();

    // Substitute parameters
    for (const DataInfo& ob : xmlDataInfoParams) {
        string placeholder = std::format(":{0}:", ob.param);
        string value = ob.value;

        // Add quotes for string and date/time types
        if (ob.type == DataInfo::String || 
            ob.type == DataInfo::DateTime || 
            ob.type == DataInfo::Date || 
            ob.type == DataInfo::Time) {
            value = (value != "NULL") ? ("'" + value + "'") : "NULL";
        }

        // Replace all occurrences
        size_t pos = 0;
        while ((pos = m_sqlSource.find(placeholder, pos)) != string::npos) {
            m_sqlSource.replace(pos, placeholder.length(), value);
            pos += value.length();
        }
    }
    
    m_sqlSource = Trimmer::trim(m_sqlSource);
    m_isParsed = true;
}
