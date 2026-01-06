#pragma

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include "company.grpc.pb.h"

#include "sqlapplet.h"
#include "sqlcommand.h"
#include "sqlconnection.h"
#include "sqlquery.h"
#include "JsonParameterFormatter.h"
#include "include_backend_util.h"
#include <easylogging++.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

using std::string;

// Service
using CompanyEdit::CompanyEditor;
// Objects (messages)
using CompanyEdit::Company;
using CompanyEdit::CompanyResult;
using CompanyEdit::CompanyList;
using CompanyEdit::JsonParameters;
using CompanyEdit::CompanyUid;
using CompanyEdit::TotalCount;

//ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

// Logic and data behind the server's behavior.
class CompanyServiceImpl final : public CompanyEditor::Service {
    Status AddCompany(ServerContext * context, const Company * company,
                    CompanyResult * result) override {

        SqlConnection con;
        std::string sqlStd;
        try {
            SQLApplet applet("company_insert.xml");
            applet.addParameter("SERVER_UID", (int)company->server_uid());
            applet.addParameter("COMPANY_TYPE", (int)company->company_type());
            applet.addParameter("NAME", company->name().c_str());
            applet.addParameter("ADDRESS", company->address().c_str());
            applet.addParameter("REG_DATE", std::chrono::milliseconds(std::chrono::milliseconds(company->reg_date())), DataInfo::Date);
            applet.addParameter("JOINT_DATE", std::chrono::milliseconds(std::chrono::milliseconds(company->joint_date())), DataInfo::Date);
            applet.addParameter("LICENSE", company->license().c_str());
            applet.parse();
            // To log if eror
            sqlStd = applet.sql();

            SAString sql(sqlStd.c_str());
            SACommand cmd(con.connectionSa(),  sql);
            con.connect();
            con.setAutoCommit(true);
            cmd.Param(_TSA("logo")).setAsLongBinary() = SaBinary::toSaString(company->logo());
            cmd.Execute();

            if(cmd.isResultSet()) {
                cmd.FetchNext();
                result->set_uid(cmd.Field("UID").asString().GetMultiByteChars());
                result->set_success(true);
                result->set_error("No error");
            } else {
                result->set_success(false);
                result->set_error("No result set");
                result->set_uid("");
            }
        } catch(SAException & x) {
            LOG(ERROR) << x.ErrText().GetMultiByteChars();
            LOG(INFO) << sqlStd;
            try {
                con.rollback();
            }
            catch(SAException &)
            {
            }
            result->set_success(false);
            result->set_error(x.ErrText().GetMultiByteChars());
            result->set_uid("");
            return Status::CANCELLED;
        } catch(const SQLAppletException & e) {
            LOG(ERROR) << e.what();
            result->set_success(false);
            result->set_error(e.what());
            result->set_uid("");
            return Status(StatusCode::INTERNAL, e.what());
        } catch(...) {
            LOG(ERROR) << "Unknown error!";
            return Status(StatusCode::ABORTED, "Unknown error!");
        }

        return Status::OK;
    }

    Status EditCompany(ServerContext * context, const Company * company,
                      CompanyResult * result) override {
        std::cout<<"EditCompany: runned"<<std::endl;

        SqlConnection con;
        std::string sqlStd;
        try {
            SQLApplet applet("company_update.xml");
            applet.addParameter("UID", company->uid().c_str());
            applet.addParameter("SERVER_UID", (int)company->server_uid());
            applet.addParameter("COMPANY_TYPE", (int)company->company_type());
            applet.addParameter("NAME", company->name().c_str());
            applet.addParameter("ADDRESS", company->address().c_str());
            applet.addParameter("REG_DATE", std::chrono::milliseconds(std::chrono::milliseconds(company->reg_date())), DataInfo::Date);
            applet.addParameter("JOINT_DATE", std::chrono::milliseconds(std::chrono::milliseconds(company->joint_date())), DataInfo::Date);
            applet.addParameter("LICENSE", company->license().c_str());
            applet.parse();
            // To log if eror
            sqlStd = applet.sql();

            SAString sql(sqlStd.c_str());
            SACommand cmd(con.connectionSa(),  sql);
            con.connect();
            con.setAutoCommit(true);
            std::string logo = company->logo();
            cmd.Param(_TSA("logo")).setAsLongBinary() = SaBinary::toSaString(company->logo());
            cmd.Execute();

            if(cmd.isResultSet()) {
                cmd.FetchNext();

                result->set_uid(cmd.Field("UID").asString().GetMultiByteChars());
                result->set_success(true);
                result->set_error("No error");
            } else {
                result->set_success(false);
                result->set_error("No result set");
                result->set_uid("");
            }
        } catch(SAException & x) {
            LOG(ERROR) << x.ErrText().GetMultiByteChars();
            LOG(INFO) << sqlStd;
            try {
                con.rollback();
            }
            catch(SAException &)
            {
            }
            result->set_success(false);
            result->set_error(x.ErrText().GetMultiByteChars());
            result->set_uid("");
            return Status::CANCELLED;
        } catch(const SQLAppletException & e) {
            LOG(ERROR) << e.what();
            result->set_success(false);
            result->set_error(e.what());
            result->set_uid("");
            return Status(StatusCode::INTERNAL, e.what());
        } catch(...) {
            LOG(ERROR) << "Unknown error!";
            return Status(StatusCode::ABORTED, "Unknown error!");
        }

        return Status::OK;
    }

    Status DeleteCompany(ServerContext * context, const Company * company,
                      CompanyResult * result) override {

        SqlConnection con;
        SqlCommand command(con, "company_delete.xml");
        try {
            con.connect();
            con.setAutoCommit(true);

            command.addParameter("UID", company->uid().c_str());

            command.execute();

            if(command.isResultSet()) {
                command.FetchNext();

                result->set_uid(command.Field("UID").asString().GetMultiByteChars());
                result->set_success(true);
                result->set_error("No error");
            } else {
                result->set_success(false);
                result->set_error("No result set");
                result->set_uid("");
            }
        } catch(SAException & x) {
            LOG(ERROR) << x.ErrText().GetMultiByteChars();
            LOG(INFO) << command.sql();
            try {
                con.rollback();
            }
            catch(SAException &)
            {
            }
            result->set_success(false);
            result->set_error(x.ErrText().GetMultiByteChars());
            result->set_uid("");
            return Status::CANCELLED;
        } catch(const SQLAppletException & e) {
            LOG(ERROR) << e.what();
            result->set_success(false);
            result->set_error(e.what());
            result->set_uid("");
            return Status(StatusCode::INTERNAL, e.what());
        } catch(...) {
            LOG(ERROR) << "Unknown error!";
            return Status(StatusCode::ABORTED, "Unknown error!");
        }

        return Status::OK;
    }

    Status QueryCompanies(ServerContext * context, const JsonParameters * params, CompanyList * list) override {
        SqlConnection con;
        SqlQuery cmd(con, "company_select.xml", JsonParameterFormatter::fromJsonString(params->jsonparams()));
        try {
            con.connect();
            while(cmd.query()) {
                Company * cp = list->add_companies();
                SAString address = cmd.Field("ADDRESS").asString();
                address.TrimRight();
                SAString license = cmd.Field("LICENSE").asString();
                license.TrimRight();

                string strRegDate = cmd.Field("REG_DATE").asString().GetMultiByteChars();
                string strJointDate = cmd.Field("JOINT_DATE").asString().GetMultiByteChars();

                std::chrono::milliseconds secRegDate = TimeFormatHelper::stringTochronoSysSec(strRegDate, DataInfo::Date);
                std::chrono::milliseconds secJointDate = TimeFormatHelper::stringTochronoSysSec(strJointDate, DataInfo::Date);

                cp->set_uid(cmd.Field("UID").asString().GetMultiByteChars());
                cp->set_server_uid(cmd.Field("SERVER_UID"));
                cp->set_company_type(cmd.Field("COMPANY_TYPE"));
                cp->set_name(cmd.Field("NAME").asString().GetMultiByteChars());
                cp->set_address(address.GetMultiByteChars());
                cp->set_reg_date(secRegDate.count());
                cp->set_joint_date(secJointDate.count());
                cp->set_license(license.GetMultiByteChars());
                cp->set_logo(cmd.Field("LOGO").asString().GetMultiByteChars());
            }
        } catch(SAException & x) {
            LOG(ERROR) << x.ErrText().GetMultiByteChars();
            LOG(INFO) << cmd.sql();
            try {
                con.rollback();
            }
            catch(SAException &)
            {
            }
            return Status::CANCELLED;
        } catch(const SQLAppletException & e) {
            LOG(ERROR) << e.what();
            return Status(StatusCode::INTERNAL, e.what());
        } catch(...) {
            LOG(ERROR) << "Unknown error!";
            return Status(StatusCode::ABORTED, "Unknown error!");
        }



        return Status::OK;
    }

    Status QueryCompanyByUid(ServerContext * context, const CompanyUid * request, Company * response) override
    {
        SqlConnection con;
        SqlQuery cmd(con, "company_select_by_uid.xml");
        try {
            con.connect();
            cmd.addParameter("UID", request->uid().c_str());
            if(cmd.query()) {
                SAString address = cmd.Field("ADDRESS").asString();
                address.TrimRight();
                SAString license = cmd.Field("LICENSE").asString();
                license.TrimRight();

                string strRegDate = cmd.Field("REG_DATE").asString().GetMultiByteChars();
                string strJointDate = cmd.Field("JOINT_DATE").asString().GetMultiByteChars();

                std::chrono::milliseconds secRegDate = TimeFormatHelper::stringTochronoSysSec(strRegDate, DataInfo::Date);
                std::chrono::milliseconds secJointDate = TimeFormatHelper::stringTochronoSysSec(strJointDate, DataInfo::Date);

                response->set_uid(cmd.Field("UID").asString().GetMultiByteChars());
                response->set_server_uid(cmd.Field("SERVER_UID"));
                response->set_company_type(cmd.Field("COMPANY_TYPE"));
                response->set_name(cmd.Field("NAME").asString().GetMultiByteChars());
                response->set_address(address.GetMultiByteChars());
                response->set_reg_date(secRegDate.count());
                response->set_joint_date(secJointDate.count());
                response->set_license(license.GetMultiByteChars());
                SAString  blobString = cmd.Field("LOGO").asBLob();
                response->set_logo(SaBinary::toStdString(blobString));

            } else {
                return Status(StatusCode::NOT_FOUND, "No record found");
            }
        } catch(SAException & x) {
            LOG(ERROR) << x.ErrText().GetMultiByteChars();
            LOG(INFO) << cmd.sql();
            try {
                con.rollback();
            }
            catch(SAException &)
            {
            }
            return Status::CANCELLED;
        } catch(const SQLAppletException & e) {
            LOG(ERROR) << e.what();
            return Status(StatusCode::INTERNAL, e.what());
        } catch(...) {
            LOG(ERROR) << "Unknown error!";
            return Status(StatusCode::ABORTED, "Unknown error!");
        }

        return Status::OK;
    }

    Status QueryCompanyTotalCount(ServerContext * context, const JsonParameters * request, TotalCount * response) override {
        SqlConnection con;
        SqlQuery cmd(con, "company_count.xml", JsonParameterFormatter::fromJsonString(request->jsonparams()));
        try {
            con.connect();
            if(cmd.query()) {
                response->set_count(cmd.Field("ROW_COUNT").asInt64());
            } else {
                return Status(StatusCode::NOT_FOUND, "No record found");
            }
        } catch(SAException & x) {
            LOG(ERROR) << x.ErrText().GetMultiByteChars();
            LOG(INFO) << cmd.sql();
            try {
                con.rollback();
            }
            catch(SAException &)
            {
            }
            return Status::CANCELLED;
        } catch(const SQLAppletException & e) {
            LOG(ERROR) << e.what();
            return Status(StatusCode::INTERNAL, e.what());
        } catch(...) {
            LOG(ERROR) << "Unknown error!";
            return Status(StatusCode::ABORTED, "Unknown error!");
        }

        return Status::OK;
    }
};

void RunCompanyServer(uint16_t port) {
    std::string server_address = absl::StrFormat("127.0.0.1:%d", port);
    CompanyServiceImpl service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}
/*
int main() {
    RunServerCompany(12345);
    return 0;
}
*/
