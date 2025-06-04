#include <grpcpp/grpcpp.h>

#include <memory>
#include <string>

#include "absl/flags/flag.h"

#include "company.grpc.pb.h"

//#include <easylogging++.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::StatusCode;

// Service
using CompanyEdit::CompanyEditor;
// Objects (messages)
using CompanyEdit::Company;
using CompanyEdit::CompanyResult;
using CompanyEdit::CompanyList;
using CompanyEdit::JsonParameters;
using CompanyEdit::CompanyUid;
using CompanyEdit::TotalCount;
using CompanyEdit::ServerUid;

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

    Status  AddCompany(const Company & company, CompanyResult & result) {
        ClientContext context;
        return stub_->AddCompany(&context, company, &result);
    }

    Status EditCompany(const Company & company, CompanyResult & result) {
        ClientContext context;
        return stub_->EditCompany(&context, company, &result);
    }

    Status DeleteCompany(const Company & company, CompanyResult & result) {
        ClientContext context;
        return stub_->DeleteCompany(&context, company, &result);
    }

    Status QueryCompanies(const JsonParameters & parameters, std::vector<Company> object_list, CompanyResult & result) {
        ClientContext context;
        CompanyList list;

        Status status = stub_->QueryCompanies(&context, parameters, &list);
        if (status.ok()) {
            for (const auto & object : list.companies()) {
                object_list.push_back(object);
            }
        }
        return status;
    }

    Status QueryCompanyByUid(const CompanyUid & uid, Company & result) {
        ClientContext context;
        return stub_->QueryCompanyByUid(&context, uid, &result);
    }
    Status QueryCompanyTotalCount(const ServerUid & uid, TotalCount & result) {
        ClientContext context;
        return stub_->QueryCompanyTotalCount(&context, uid, &result);
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
