#include "../configfile.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {
std::string const projectPath = "";
std::string const project = "TestProject";
}

// Test suite for the Calculator class
TEST(ConfigFileTests, LoadTest)
{
    ConfigFile * config = ConfigFile::Instance();
    config->projectPath();
}


