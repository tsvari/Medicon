#include "../configfile.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(ConfigFileTests, WrongPath)
{
    try {
        ConfigFile * config = ConfigFile::InstanceCustom("", "");
    } catch(const std::invalid_argument & err) {
        // and this tests that it has the correct message
        EXPECT_STREQ(CONFIG_ERR_ALL_PROJECT_PATH.c_str(), err.what());
    } catch (...) {
        FAIL() << "Expected a different exception type.";
    }
}


TEST(ConfigFileTests, WrongPathToConfig)
{
    try {
        ConfigFile * config = ConfigFile::InstanceCustom(ALL_PROJECT_PATH, "");
    } catch(const std::invalid_argument & err) {
        // and this tests that it has the correct message
        EXPECT_STREQ(CONFIG_ERR_CONFIG_FILE.c_str(), err.what());
    } catch (...) {
        FAIL() << "Expected a different exception type.";
    }
}

TEST(ConfigFileTests, RightPathToConfigAndWrongProjectName)
{
    try {
        ConfigFile * config = ConfigFile::InstanceCustom(ALL_PROJECT_TEST_PATH, "WrongProject");
    } catch(const std::invalid_argument & err) {
        // and this tests that it has the correct message
        EXPECT_STREQ(CONFIG_ERR_CONFIG_FILE.c_str(), err.what());
    } catch (...) {
        FAIL() << "Expected a different exception type.";
    }
}

TEST(ConfigFileTests, WrongLogFilePath)
{
    try {
        ConfigFile * config = ConfigFile::InstanceCustom(ALL_PROJECT_TEST_PATH, PROJECT_NAME);
    } catch(const std::invalid_argument & err) {
        // and this tests that it has the correct message
        EXPECT_STREQ(CONFIG_ERR_LOG_FILE.c_str(), err.what());
    } catch (...) {
        FAIL() << "Expected a different exception type.";
    }
}


