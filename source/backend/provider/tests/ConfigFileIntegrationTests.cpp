#include "configfile.h"
#include "gtest/gtest.h"

TEST(ConfigFileTests, LoadAndCheckData)
{
    ConfigFile * config = nullptr;
    EXPECT_NO_THROW(config = ConfigFile::Instance());
    EXPECT_TRUE(config->load());

    string alProjectPath(ALL_PROJECT_APPDATA_PATH);

    string projectPath = alProjectPath + string("provider/");
    string templatetPath = alProjectPath + string("provider/templates/");
    string logFilePath = alProjectPath + string("provider/log/provider.log");
    string appletPath = alProjectPath + string("provider/sql-applets/");

    EXPECT_EQ(config->appletPath(), appletPath);
    EXPECT_EQ(config->templatetPath(), templatetPath);
    EXPECT_EQ(config->logFilePath(), logFilePath);
    EXPECT_EQ(config->projectPath(), projectPath);

    EXPECT_NO_THROW(config->value("host"));
    EXPECT_NO_THROW(config->value("user"));
    EXPECT_NO_THROW(config->value("pass"));
    EXPECT_THROW(config->value("WrongKey"), std::out_of_range);
}





