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
#include "sqlconnection.h"
#include <easylogging++.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

// Service
using CompanyEdit::CompanyEditor;
// Objects (messages)
using CompanyEdit::Company;
using CompanyEdit::CompanyResult;
using CompanyEdit::CompanyList;
using CompanyEdit::XmlParameters;

//ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

// Logic and data behind the server's behavior.
class CompanyServiceImpl final : public CompanyEditor::Service {
    Status AddCompany(ServerContext * context, const Company * company,
                    CompanyResult * result) override {
        std::cout<<"AddCompany: runned"<<std::endl;

        result->set_success(true);
        result->set_error("No error");
        result->set_uid(company->uid());

        return Status::OK;
    }

    Status EditCompany(ServerContext * context, const Company * company,
                      CompanyResult * result) override {
        std::cout<<"EditCompany: runned"<<std::endl;

        result->set_success(true);
        result->set_error("No error");
        result->set_uid(company->uid());

        return Status::OK;
    }

    Status DeleteCompany(ServerContext * context, const Company * company,
                      CompanyResult * result) override {

        std::cout<<"DeleteCompany: runned"<<std::endl;

        result->set_success(true);
        result->set_error("No error");
        result->set_uid(company->uid());

        return Status::OK;
    }

    Status QueryCompanies(ServerContext * context, const XmlParameters * params, CompanyList * list) override {
        std::cout<<"QueryCompanies: runned"<<std::endl;
        SqlConnection con;
        try
        {
            con.connect();
            SACommand select(con.connectionSa(), _TSA(""));
            std::cout<<"Connected well"<<std::endl;
            //con.Disconnect();

        } catch(SAException & x) {
            // SAConnection::Rollback()
            // can also throw an exception
            // (if a network error for example),
            // we will be ready
            std::cout<<"Error with connection!"<<std::endl;
            try
            {
                // on error rollback changes
                con.rollback();
            } catch(SAException &) {

            }
            // print error message
            //LOG(ERROR) << "DBConnection: " << x.ErrText().GetMultiByteChars();
        }

        for(int it = 0; it < 10; it ++) {
            Company * cp = list->add_companies();
            cp->set_uid(std::to_string(it));
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
