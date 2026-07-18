#ifndef COMPANY_SERVER_H
#define COMPANY_SERVER_H

#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>

#include "company.grpc.pb.h"
#include "company_service.h"
#include "company_types.h"

// Required in header for method signatures and base class
using grpc::ServerContext;
using grpc::Status;
using CompanyEdit::CompanyEditor;
using CompanyEdit::Company;
using CompanyEdit::CompanyResult;
using CompanyEdit::CompanyList;
using CompanyEdit::JsonParameters;
using CompanyEdit::CompanyUid;
using CompanyEdit::TotalCount;

/**
 * @brief Thin gRPC adapter — delegates all work to CompanyService
 *
 * Converts protobuf requests to domain types, calls CompanyService,
 * and converts results back to protobuf responses.
 */
class CompanyServiceImpl final : public CompanyEditor::Service {
public:
    CompanyServiceImpl(std::unique_ptr<CompanyService> service, bool logSql);

    // 6 gRPC overrides
    Status AddCompany(ServerContext* context, const Company* company,
                      CompanyResult* result) override;

    Status EditCompany(ServerContext* context, const Company* company,
                       CompanyResult* result) override;

    Status DeleteCompany(ServerContext* context, const Company* company,
                         CompanyResult* result) override;

    Status QueryCompanies(ServerContext* context, const JsonParameters* params,
                          CompanyList* list) override;

    Status QueryCompanyByUid(ServerContext* context, const CompanyUid* request,
                             Company* response) override;

    Status QueryCompanyTotalCount(ServerContext* context,
                                  const JsonParameters* request,
                                  TotalCount* response) override;

private:
    // Protobuf ↔ domain type conversion helpers
    static CompanyData toCompanyData(const Company& company);
    static CompanyFilter toCompanyFilter(const JsonParameters& params);
    static void toProto(const CompanyData& data, Company* proto);

    void logError(const char* op, const std::string& detail) const;

    std::unique_ptr<CompanyService> m_service;
    bool m_logSql = false;
};

// Server entry point — wires layers and starts gRPC server
void RunCompanyServer(uint16_t port, bool logSql,
                      const std::string& appletPath,
                      const std::string& dbHost,
                      const std::string& dbUser,
                      const std::string& dbPass);

#endif // COMPANY_SERVER_H
