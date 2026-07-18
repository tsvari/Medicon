/**
 * @file test_main.cpp
 * @brief Initializes easylogging++ storage for integration test targets
 *
 * Required by provider_lib (company_repository) which uses LOG_IF for SQL logging.
 * Without INITIALIZE_EASYLOGGINGPP, the el::base::elStorage symbol is undefined.
 *
 * Does NOT define main() — gmock_main.cc provides that.
 */
#include <easylogging++.h>
INITIALIZE_EASYLOGGINGPP
