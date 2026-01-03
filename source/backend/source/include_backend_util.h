#pragma once

#include "include_util.h"
#include "configfile.h"

#include <SQLAPI.h>

/**
 * @brief Binary data conversion utilities for SQLAPI
 * 
 * Provides conversion functions between:
 * - std::string and SAString (SQLAPI's string type)
 * - Binary files and both string types
 * 
 * Useful for handling BLOB data, binary files, and SQLAPI interoperability.
 */
namespace SaBinary {
/**
 * @brief Load binary file into std::string
 * @param pathToBinary Path to file
 * @return File contents as std::string
 * @throws std::system_error if file cannot be opened
 */
std::string toStdString(const char * pathToBinary);

/**
 * @brief Convert SAString to std::string
 * @param saString SQLAPI string with binary data
 * @return Binary data as std::string
 */
std::string toStdString(const SAString & saString);

/**
 * @brief Load binary file into SAString
 * @param pathToBinary Path to file
 * @return File contents as SAString
 * @throws std::system_error if file cannot be opened
 */
SAString toSaString(const char * pathToBinary);

/**
 * @brief Convert std::string to SAString
 * @param stdString Binary data as std::string
 * @return Binary data as SAString
 */
SAString toSaString(const std::string & stdString);
}


