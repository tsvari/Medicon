#include "configfile.h"
#include "gmock/gmock.h"

class ConfigFileMock : public ConfigFile
{
public:
    explicit ConfigFileMock(const char* allProjectPath, const  char *  projectName):
        ConfigFile(allProjectPath, projectName){}
    MOCK_METHOD0(load, bool());
};
