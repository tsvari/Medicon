#pragma

#include "include_util.h"
#include "configfile.h"

#include <SQLAPI.h>

#ifdef Q_OS_MAC
//#define PROJECT_PATH "/Users/tsvari/tida-web/"
//#elif Q_OS_LINUX
//#define PROJECT_PATH "/home/vaky/tida-web/"
#endif

namespace SaBinary {
std::string toStdString(const char * pathToBinary);
std::string toStdString(const SAString & saString);
SAString toSaString(const char * pathToBinary);
SAString toSaString(const std::string & stdString);
}


