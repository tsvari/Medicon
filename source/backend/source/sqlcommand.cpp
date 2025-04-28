#include "sqlcommand.h"
#include "sqlapplet.h"

#include <filesystem>
#include <cassert>

SQLCommand::SQLCommand(const char * applet_name) :
    executed_(false),
    last_inserted_id_(0)
{  
    applet_path_ = (qConfigFile.appletPath() + string(applet_name));
    assert(std::filesystem::exists(applet_path_));
}

SQLCommand::~SQLCommand()
{
}

void SQLCommand::SetApplet(const char * applet_name)
{
    string xmlData = "";
    if(xml_gen_)
        xmlData = xml_gen_->GenerateXmlData();

    SQLApplet applet((applet_path_+ string(applet_name)).c_str(), xmlData.c_str());
    applet.Read();

    command_text_ = Trim(applet.GetSource());
}

void SQLCommand::SetSql(const char *sql)
{
    command_text_ = sql;

    if(!xml_gen_)
        return;

    const vector<DataInfo> params = xml_gen_->GetData();
    for( unsigned int i = 0; i < params.size(); i++ )
        {
        DataInfo ob = params.at(i);
        string sTo = ":" + ob._tag_name;

        if(ob._tag_type == GlobalType::String )
            {
            ob._tag_value = "'" + ob._tag_value + "'";
            }
        else if(ob._tag_type == GlobalType::DateTime || ob._tag_type == GlobalType::Date || ob._tag_type == GlobalType::Time)
            {
            if(	ob._tag_value != string("NULL") )
                ob._tag_value = "'" + ob._tag_value + "'";
            }

        size_t start_pos = command_text_.find(sTo);
        while(start_pos && start_pos < command_text_.length())
            {
            command_text_.replace(start_pos, sTo.length(), ob._tag_value);
            start_pos = command_text_.find(sTo, start_pos);
            }
        }
}

void SQLCommand::Execute(bool last_insert_id)
{
    if(!conn_)
        throw sql::SQLException("Connection was not init!");

    if(!stmt_)
        stmt_ = conn_->createStatement();

    stmt_->execute(command_text_);
    if(last_insert_id)
        {
        res_ = stmt_->executeQuery("SELECT LAST_INSERT_ID() AS id");
        res_->next();
        last_inserted_id_ = res_->getInt64("id");
        }

}

bool SQLCommand::Query()
{
    if(!conn_)
        throw sql::SQLException("Connection was not init!");

    if(!executed_)
        {
        stmt_ = conn_->createStatement();
        res_ = stmt_->executeQuery(command_text_);
        executed_ = true;
        }
    return res_->next();
}

string SQLCommand::Trim(const string &str)
{
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last-first+1));
}

void AddDbInfo(const char * tgName, const char * tgValue, unsigned int nType)
{

}
void AddDbInfo(const char * tgName, const string & tgValue, unsigned int nType)
{

}
void AddDbInfo(const char * tgName, const bool tgValue, unsigned int nType)
{

}
void AddDbInfo(const char * tgName, const double tgValue, unsigned int nType)
{

}
void AddDbInfo(const char * tgName, const int tgValue, unsigned int nType)
{

}
void AddDbInfo(const char * tgName, const time_t tgValue, unsigned int nType)
