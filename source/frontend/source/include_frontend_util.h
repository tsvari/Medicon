#pragma

#include "include_util.h"

#include <QString>

#ifdef Q_OS_MAC
//#define PROJECT_PATH "/Users/tsvari/tida-web/"
//#elif Q_OS_LINUX
//#define PROJECT_PATH "/home/vaky/tida-web/"
#endif

namespace FrontConverter {
QString to_str(const std::string & source);
std::string to_str(const QString & source);
std::string to_str(const QVariant & source);
}




