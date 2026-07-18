/**
 * @file test_main.cpp
 * @brief Initializes easylogging++ storage for test targets
 *
 * Required by company_repository.cpp which uses LOG_IF for SQL logging.
 * Without INITIALIZE_EASYLOGGINGPP, the el::base::elStorage symbol is
 * undefined, causing a linker error.
 *
 * Does NOT define main() — gmock_main.cc provides that. Only initializes
 * the logging storage singleton so LOG_IF compiles and links.
 */
#include <easylogging++.h>
INITIALIZE_EASYLOGGINGPP
