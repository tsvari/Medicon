#include <grpcpp/grpcpp.h>

#include <memory>
#include <string>

#include "absl/flags/flag.h"

#include "company.grpc.pb.h"

//#include <easylogging++.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

// Service
using CompanyEdit::CompanyEditor;
// Objects (messages)
using CompanyEdit::Company;
using CompanyEdit::CompanyResult;
using CompanyEdit::CompanyList;
using CompanyEdit::XmlParameters;

ABSL_FLAG(std::string, target, "0.0.0.0:12345", "Server address");

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class CompanyEditorClient
{
public:
    CompanyEditorClient(std::shared_ptr<Channel> channel)
        : stub_(CompanyEditor::NewStub(channel))
    {}

    void  AddCompany(const Company & company, CompanyResult & result) {
        std::cout<< "Start AddCompany" << std::endl;
        ClientContext context;
        Status status = stub_->AddCompany(&context, company, &result);
        std::cout<< to_string(status.error_code()) + std::string(": ") + status.error_message() << std::endl;
        if(!status.ok()) {
            result.set_error(to_string(status.error_code()) + std::string(": ") + status.error_message());
            result.set_success(false);
            result.set_uid("-1");
        } else {
            result.set_success(true);
            result.set_uid(result.uid());
        }
    }

    void EditCompany(const Company & company, CompanyResult & result) {
        ClientContext context;
        Status status = stub_->EditCompany(&context, company, &result);
        if(!status.ok()) {
            result.set_error(to_string(status.error_code()) + std::string(": ") + status.error_message());
            result.set_success(false);
            result.set_uid("-1");
        } else {
            result.set_success(true);
            result.set_uid(company.uid());
        }
    }

    void DeleteCompany(const Company & company, CompanyResult & result) {
        ClientContext context;
        Status status = stub_->DeleteCompany(&context, company, &result);
        if(!status.ok()) {
            result.set_error(to_string(status.error_code()) + std::string(": ") + status.error_message());
            result.set_success(false);
            result.set_uid("-1");
        } else {
            result.set_success(true);
            result.set_uid(company.uid());
        }
    }

    void QueryCompanies(const XmlParameters & parameters, std::vector<Company> object_list, CompanyResult & result) {
        ClientContext context;
        CompanyList list;

        Status status = stub_->QueryCompanies(&context, parameters, &list);
        if (status.ok()) {
            for (const auto & object : list.companies()) {
                object_list.push_back(object);
            }
            result.set_success(true);
        } else {
            result.set_error(to_string(status.error_code()) + std::string(": ") + status.error_message());
            result.set_success(false);
        }
    }

private:
    std::unique_ptr<CompanyEditor::Stub> stub_;
};
/*
int main(int argc, char** argv) {
    ObjectClient client(grpc::CreateChannel(
        "localhost:50051", grpc::InsecureChannelCredentials()));
    std::vector<Object> objects = client.ListObjects();

    for (const auto& object : objects) {
        std::cout << "Object ID: " << object.id() << ", Name: " << object.name()
        << std::endl;
    }

    return 0;
}
*/
/*
int main(int argc, char** argv) {
    std::cout << "Start Client" << std::endl;
    CompanyEditorClient client(grpc::CreateChannel("127.0.0.1:12345", grpc::InsecureChannelCredentials()));

    Company company;
    company.set_uid("11");
    CompanyResult result;

    client.AddCompany(company, result);

    std::cout << "Error: " << result.error() << std::endl;
    std::cout << "uid: " << result.uid() << std::endl;

    return 0;
}
*/
