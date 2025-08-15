#include "include_backend_util.h"
#include <fstream>
#include <iostream>
#include <cstring>

namespace SaBinary {

std::string toStdString(const char * pathToBinary) {
    return StdBinary::toStdString(pathToBinary);
}

std::string toStdString(const SAString & saString) {
    const void * saBinaryBuffer = saString.GetBinaryData();
    size_t saBufferLength = saString.GetBinaryLength();
    return std::string(static_cast<const char*>(saBinaryBuffer), saBufferLength);
}

SAString toSaString(const char * pathToBinary) {
    std::string stdString = toStdString(pathToBinary);
    return toSaString(stdString);
}

SAString toSaString(const std::string & stdString) {
    SAString saString;
    void * stdBuffer = saString.GetBinaryBuffer(stdString.size());
    std::memcpy(stdBuffer, stdString.c_str(), stdString.size());
    saString.ReleaseBinaryBuffer(stdString.size());
    return saString;
}
}
