#include "sqlapplet.h"
#include "Markup.h"
#include <time.h>
#include <easylogging++.h>
#include <format>

using std::map;

namespace {
string AppletPath="";
}

SQLApplet::SQLApplet(const char * appletName, map<string, string> formattedParamValueList):
    formattedParamValueList_(formattedParamValueList)
{
    appletPath_ = std::format("{}{}", AppletPath, appletName);
    LOG(INFO) << "Applet constructor";
}

SQLApplet::SQLApplet(const char *appletName, const TypeToStringFormatter & formatter) :
    formattedParamValueList_(formatter.formattedParamValueList())
{
    appletPath_ = std::format("{}{}", AppletPath, appletName);
}

void SQLApplet::InitPath(const char * appletPath)
{
    AppletPath = appletPath;
}

void SQLApplet::parse()
{
    // check existense of applett and data content
    if(appletPath_.empty()) {
        throw SQLAppletException( "SQLApplet: No path applet name! ");
    }

    CMarkup	parser;
    // reuse parser
    if (!parser.Load(appletPath_)) {
        throw SQLAppletException(parser.GetError().c_str() );
    }

    // find Description Tag/Value
    if(!parser.FindChildElem( "Description" )) {
        throw SQLAppletException( "SQLApplet - Description-tag was not found" );
    }

    description_ = parser.GetChildData();

    // - find params and fill vector
    while (parser.FindChildElem("Param")) {
        parser.IntoElem();

        DataInfo ob;

        if (parser.FindChildElem("Name")) {
            ob.param = parser.GetChildData();
        }
        if (parser.FindChildElem("Type")) {
            // find appropriate data type
            string sType = parser.GetChildData();
            if(sType == "NUMERIC")
                ob.type = DataInfo::Int;
            else if(sType == "STRING")
                ob.type = DataInfo::String;
            else if(sType == "DATETIME")
                ob.type = DataInfo::DateTime;
            else if(sType == "DATE")
                ob.type = DataInfo::Date;
            else if(sType == "TIME")
                ob.type = DataInfo::Time;
        }

        // check param/value in data file
        if(formattedParamValueList_.find(ob.param) == formattedParamValueList_.end()) {
            if (parser.FindChildElem("DefVal"))
                ob.value = parser.GetChildData();
        } else {
            ob.value = formattedParamValueList_[ob.param];
        }

        params_.push_back(ob);
        parser.OutOfElem();
    }

    // - find SQL code
    if (!parser.FindChildElem("Code")) {
        throw SQLAppletException( "SQLApplet - SQL-script could not found!" );
    }

    sqlSource_ = parser.GetChildData();

    // replace params by value
    for( unsigned int i = 0; i < params_.size(); i++ ) {
        DataInfo ob = params_.at(i);
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

        size_t start_pos = sqlSource_.find(sTo);
        while(start_pos && start_pos < sqlSource_.length()) {
            sqlSource_.replace(start_pos, sTo.length(), ob.value);
            start_pos = sqlSource_.find(sTo, start_pos);
        }
    }
    sqlSource_ = trimLeftRight(sqlSource_);
}
