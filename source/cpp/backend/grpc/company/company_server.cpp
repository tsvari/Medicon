#include "company_server.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>
#include <absl/strings/str_format.h>

#include <iostream>
#include <memory>

#include "JsonParameterFormatter.h"
#include "include_backend_util.h"
#include <easylogging++.h>

using grpc::Server;
using grpc::ServerBuilder;

using grpc::Status;
using grpc::StatusCode;
using std::string;

// ============================================================================
// Construction
// ============================================================================

CompanyServiceImpl::CompanyServiceImpl(
    std::unique_ptr<CompanyService> service, bool logSql)
    : m_service(std::move(service))
    , m_logSql(logSql)
{
}

// ============================================================================
// Protobuf ↔ Domain type conversion
// ============================================================================

CompanyData CompanyServiceImpl::toCompanyData(const Company& company)
{
    CompanyData data;
    data.uid = company.uid();
    data.server_uid = company.server_uid();
    data.company_type = company.company_type();
    data.name = company.name();
    data.address = company.address();
    data.reg_date = std::chrono::milliseconds(company.reg_date());
    data.joint_date = std::chrono::milliseconds(company.joint_date());
    data.license = company.license();
    data.logo = company.logo();
    return data;
}

CompanyFilter CompanyServiceImpl::toCompanyFilter(const JsonParameters& params)
{
    CompanyFilter filter;
    auto map = JsonParameterFormatter::fromJsonString(params.jsonparams());
    auto it = map.find("FILTER_FIELD");
    if (it != map.end()) filter.field = it->second;
    it = map.find("FILTER_VALUE");
    if (it != map.end()) filter.value = it->second;
    it = map.find("OFFSET");
    if (it != map.end()) filter.offset = std::stoi(it->second);
    it = map.find("LIMIT");
    if (it != map.end()) filter.limit = std::stoi(it->second);
    return filter;
}

void CompanyServiceImpl::toProto(const CompanyData& data, Company* proto)
{
    proto->set_uid(data.uid);
    proto->set_server_uid(data.server_uid);
    proto->set_company_type(data.company_type);
    proto->set_name(data.name);
    proto->set_address(data.address);
    proto->set_reg_date(data.reg_date.count());
    proto->set_joint_date(data.joint_date.count());
    proto->set_license(data.license);
    proto->set_logo(data.logo);
}

// ============================================================================
// Error logging
// ============================================================================

void CompanyServiceImpl::logError(const char* op, const std::string& detail) const
{
    if (m_logSql) {
        LOG(INFO) << op << ": " << detail;
    } else {
        LOG(INFO) << op;
    }
}

// ============================================================================
// gRPC — AddCompany
// ============================================================================

Status CompanyServiceImpl::AddCompany(ServerContext*, const Company* company,
                                       CompanyResult* result)
{
    try {
        CompanyData data = toCompanyData(*company);
        CompanyData out = m_service->addCompany(data);
        result->set_uid(out.uid);
        result->set_success(true);
        result->set_error("No error");
        return Status::OK;
    } catch (const SAException& e) {
        LOG(ERROR) << e.ErrText().GetMultiByteChars();
        logError("AddCompany", "SQL error");
        result->set_success(false);
        result->set_error(e.ErrText().GetMultiByteChars());
        return Status::CANCELLED;
    } catch (const std::exception& e) {
        LOG(ERROR) << e.what();
        result->set_success(false);
        result->set_error(e.what());
        return Status(StatusCode::INTERNAL, e.what());
    } catch (...) {
        LOG(ERROR) << "Unknown error in AddCompany";
        result->set_success(false);
        result->set_error("Unknown error");
        return Status(StatusCode::ABORTED, "Unknown error!");
    }
}

// ============================================================================
// gRPC — EditCompany
// ============================================================================

Status CompanyServiceImpl::EditCompany(ServerContext*, const Company* company,
                                        CompanyResult* result)
{
    try {
        CompanyData data = toCompanyData(*company);
        CompanyData out = m_service->editCompany(data);
        result->set_uid(out.uid);
        result->set_success(true);
        result->set_error("No error");
        return Status::OK;
    } catch (const SAException& e) {
        LOG(ERROR) << e.ErrText().GetMultiByteChars();
        logError("EditCompany", "SQL error");
        result->set_success(false);
        result->set_error(e.ErrText().GetMultiByteChars());
        return Status::CANCELLED;
    } catch (const std::exception& e) {
        LOG(ERROR) << e.what();
        result->set_success(false);
        result->set_error(e.what());
        return Status(StatusCode::INTERNAL, e.what());
    } catch (...) {
        LOG(ERROR) << "Unknown error in EditCompany";
        result->set_success(false);
        result->set_error("Unknown error");
        return Status(StatusCode::ABORTED, "Unknown error!");
    }
}

// ============================================================================
// gRPC — DeleteCompany
// ============================================================================

Status CompanyServiceImpl::DeleteCompany(ServerContext*, const Company* company,
                                          CompanyResult* result)
{
    try {
        DeleteResult out = m_service->deleteCompany(company->uid());
        result->set_uid(out.uid);
        result->set_success(out.success);
        result->set_error(out.error);
        return Status::OK;
    } catch (const SAException& e) {
        LOG(ERROR) << e.ErrText().GetMultiByteChars();
        logError("DeleteCompany", "SQL error");
        result->set_success(false);
        result->set_error(e.ErrText().GetMultiByteChars());
        return Status::CANCELLED;
    } catch (const std::exception& e) {
        LOG(ERROR) << e.what();
        result->set_success(false);
        result->set_error(e.what());
        return Status(StatusCode::INTERNAL, e.what());
    } catch (...) {
        LOG(ERROR) << "Unknown error in DeleteCompany";
        result->set_success(false);
        result->set_error("Unknown error");
        return Status(StatusCode::ABORTED, "Unknown error!");
    }
}

// ============================================================================
// gRPC — QueryCompanies
// ============================================================================

Status CompanyServiceImpl::QueryCompanies(ServerContext*,
                                           const JsonParameters* params,
                                           CompanyList* list)
{
    try {
        CompanyFilter filter = toCompanyFilter(*params);
        auto results = m_service->queryCompanies(filter);
        for (const auto& data : results) {
            toProto(data, list->add_companies());
        }
        return Status::OK;
    } catch (const SAException& e) {
        LOG(ERROR) << e.ErrText().GetMultiByteChars();
        logError("QueryCompanies", "SQL error");
        return Status::CANCELLED;
    } catch (const std::exception& e) {
        LOG(ERROR) << e.what();
        return Status(StatusCode::INTERNAL, e.what());
    } catch (...) {
        LOG(ERROR) << "Unknown error in QueryCompanies";
        return Status(StatusCode::ABORTED, "Unknown error!");
    }
}

// ============================================================================
// gRPC — QueryCompanyByUid
// ============================================================================

Status CompanyServiceImpl::QueryCompanyByUid(ServerContext*,
                                              const CompanyUid* request,
                                              Company* response)
{
    try {
        auto opt = m_service->getCompanyByUid(request->uid());
        if (opt.has_value()) {
            toProto(opt.value(), response);
            return Status::OK;
        }
        return Status(StatusCode::NOT_FOUND, "No record found");
    } catch (const SAException& e) {
        LOG(ERROR) << e.ErrText().GetMultiByteChars();
        logError("QueryCompanyByUid", "SQL error");
        return Status::CANCELLED;
    } catch (const std::exception& e) {
        LOG(ERROR) << e.what();
        return Status(StatusCode::INTERNAL, e.what());
    } catch (...) {
        LOG(ERROR) << "Unknown error in QueryCompanyByUid";
        return Status(StatusCode::ABORTED, "Unknown error!");
    }
}

// ============================================================================
// gRPC — QueryCompanyTotalCount
// ============================================================================

Status CompanyServiceImpl::QueryCompanyTotalCount(ServerContext*,
                                                   const JsonParameters* request,
                                                   TotalCount* response)
{
    try {
        CompanyFilter filter = toCompanyFilter(*request);
        int64_t count = m_service->countCompanies(filter);
        response->set_count(count);
        return Status::OK;
    } catch (const SAException& e) {
        LOG(ERROR) << e.ErrText().GetMultiByteChars();
        logError("QueryCompanyTotalCount", "SQL error");
        return Status::CANCELLED;
    } catch (const std::exception& e) {
        LOG(ERROR) << e.what();
        return Status(StatusCode::INTERNAL, e.what());
    } catch (...) {
        LOG(ERROR) << "Unknown error in QueryCompanyTotalCount";
        return Status(StatusCode::ABORTED, "Unknown error!");
    }
}

// ============================================================================
// Server entry point
// ============================================================================

void RunCompanyServer(uint16_t port, bool logSql,
                      const std::string& appletPath,
                      const std::string& dbHost,
                      const std::string& dbUser,
                      const std::string& dbPass)
{
    auto service = std::make_unique<CompanyService>(
        appletPath, dbHost, dbUser, dbPass);

    CompanyServiceImpl impl(std::move(service), logSql);

    std::string server_address = absl::StrFormat("127.0.0.1:%d", port);

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&impl);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}
