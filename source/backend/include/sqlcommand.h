#ifndef SQLCOMMAND_H
#define SQLCOMMAND_H

#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "include_backend_util.h"
#include "XmlDataGenerator.h"


class SQLCommand
{
public:
    SQLCommand();
    explicit SQLCommand(const char* applet_name);
    virtual ~SQLCommand();

    void SetApplet(const char* applet_name);
    void SetSql(const char* sql);

    void Execute(bool last_insert_id=false);
    bool Query();

    inline string GetCommand(){ return command_text_; }
    inline int64_t GetLastInsertedId(){ return last_inserted_id_;}

    void AddDbInfo(const char * tgName, const char * tgValue, unsigned int nType = DataInfo::String) {
        xml_gen_.AddDbInfo(tgName, tgValue, nType);
    }
    void AddDbInfo(const char * tgName, const string & tgValue, unsigned int nType = DataInfo::String) {
        xml_gen_.AddDbInfo(tgName, tgValue, nType);
    }
    void AddDbInfo(const char * tgName, const bool tgValue, unsigned int nType = DataInfo::Int) {
        xml_gen_.AddDbInfo(tgName, tgValue, nType);
    }
    void AddDbInfo(const char * tgName, const double tgValue, unsigned int nType = DataInfo::Double) {
        xml_gen_.AddDbInfo(tgName, tgValue, nType);
    }
    void AddDbInfo(const char * tgName, const int tgValue, unsigned int nType = DataInfo::Int) {
        xml_gen_.AddDbInfo(tgName, tgValue, nType);
    }
    void AddDbInfo(const char * tgName, const time_t tgValue, unsigned int nType) {
        xml_gen_.AddDbInfo(tgName, tgValue, nType);
    }

protected:
    string Trim(const string & str);
private:
    XmlDataGenerator xml_gen_;
    string applet_path_;

    string command_text_;

    bool executed_;
    int64_t last_inserted_id_;
};

#endif // SQLCOMMAND_H
