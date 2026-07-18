/**
 * @file ConfigFIleTests.cpp
 * @brief Pure unit tests for ConfigFile singleton class
 * 
 * Tests configuration validation, error handling, and path construction.
 * Uses ConfigFileForTesting for isolated, testable instances.
 */

#include "../configfile.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

/**
 * @class ConfigFileTests
 * @brief Test fixture for ConfigFile unit tests
 */
class ConfigFileTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Pure unit tests - no setup needed
    }

    void TearDown() override {
        // No cleanup needed
    }
};

// ============================================================================
// Constructor and Path Validation Tests
// ============================================================================

/**
 * @test Verify empty path throws exception
 */
TEST_F(ConfigFileTests, Constructor_EmptyPath_ThrowsException)
{
    EXPECT_THROW({
        ConfigFile::InstanceCustom("", "");
    }, std::invalid_argument);
}

/**
 * @test Verify nullptr path throws exception
 */
TEST_F(ConfigFileTests, Constructor_NullPath_ThrowsException)
{
    EXPECT_THROW({
        ConfigFile::InstanceCustom(nullptr, "TestProject");
    }, std::invalid_argument);
}

/**
 * @test Verify empty project name throws exception
 */
TEST_F(ConfigFileTests, Constructor_EmptyProjectName_ThrowsException)
{
    EXPECT_THROW({
        ConfigFile::InstanceCustom(ALL_PROJECT_TEST_APPDATA_PATH, "");
    }, std::invalid_argument);
}

/**
 * @test Verify nullptr project name throws exception
 */
TEST_F(ConfigFileTests, Constructor_NullProjectName_ThrowsException)
{
    EXPECT_THROW({
        ConfigFile::InstanceCustom(ALL_PROJECT_TEST_APPDATA_PATH, nullptr);
    }, std::invalid_argument);
}

/**
 * @test Verify non-existent directory throws exception
 */
TEST_F(ConfigFileTests, Constructor_NonExistentPath_ThrowsException)
{
    EXPECT_THROW({
        ConfigFile::InstanceCustom("/non/existent/path/", "TestProject");
    }, std::invalid_argument);
}

/**
 * @test Verify wrong project name throws config file not found
 */
TEST_F(ConfigFileTests, Constructor_WrongProjectName_ThrowsException)
{
    EXPECT_THROW({
        ConfigFile::InstanceCustom(ALL_PROJECT_TEST_APPDATA_PATH, "NonExistentProject");
    }, std::invalid_argument);
}

/**
 * @test Verify missing log file throws exception
 */
TEST_F(ConfigFileTests, Constructor_MissingLogFile_ThrowsException)
{
    // GlobalTestProject.json exists but log/ directory doesn't
    EXPECT_THROW({
        ConfigFile::InstanceCustom(ALL_PROJECT_TEST_APPDATA_PATH, PROJECT_NAME);
    }, std::invalid_argument);
}

// ============================================================================
// Path Getter Tests (using ConfigFileForTesting for testability)
// ============================================================================

/**
 * @test Verify appletPath() returns correct path
 */
TEST_F(ConfigFileTests, AppletPath_ReturnsCorrectPath)
{
    // Create temp structure for testing
    fs::path tempDir = fs::temp_directory_path() / "config_path_test";
    fs::path projectDir = tempDir / "TestProject";
    fs::path logDir = projectDir / "log";
    
    fs::remove_all(tempDir);
    fs::create_directories(logDir);
    
    // Create minimal required files
    std::ofstream(projectDir / "TestProject.json") << "{}";
    std::ofstream(logDir / "TestProject.log") << "";
    
    ConfigFileForTesting config(tempDir.string().c_str(), "TestProject");
    
    std::string expected = tempDir.string() + "/TestProject/sql-applets/";
    EXPECT_EQ(config.appletPath(), expected);
    
    fs::remove_all(tempDir);
}

/**
 * @test Verify templatePath() returns correct path
 */
TEST_F(ConfigFileTests, TemplatePath_ReturnsCorrectPath)
{
    fs::path tempDir = fs::temp_directory_path() / "config_path_test2";
    fs::path projectDir = tempDir / "TestProject";
    fs::path logDir = projectDir / "log";
    
    fs::remove_all(tempDir);
    fs::create_directories(logDir);
    
    std::ofstream(projectDir / "TestProject.json") << "{}";
    std::ofstream(logDir / "TestProject.log") << "";
    
    ConfigFileForTesting config(tempDir.string().c_str(), "TestProject");
    
    std::string expected = tempDir.string() + "/TestProject/templates/";
    EXPECT_EQ(config.templatePath(), expected);
    
    fs::remove_all(tempDir);
}

/**
 * @test Verify logFilePath() returns correct path
 */
TEST_F(ConfigFileTests, LogFilePath_ReturnsCorrectPath)
{
    fs::path tempDir = fs::temp_directory_path() / "config_path_test3";
    fs::path projectDir = tempDir / "TestProject";
    fs::path logDir = projectDir / "log";
    
    fs::remove_all(tempDir);
    fs::create_directories(logDir);
    
    std::ofstream(projectDir / "TestProject.json") << "{}";
    std::ofstream(logDir / "TestProject.log") << "";
    
    ConfigFileForTesting config(tempDir.string().c_str(), "TestProject");
    
    std::string expected = tempDir.string() + "/TestProject/log/TestProject.log";
    EXPECT_EQ(config.logFilePath(), expected);
    
    fs::remove_all(tempDir);
}

/**
 * @test Verify configFilePath() returns correct path
 */
TEST_F(ConfigFileTests, ConfigFilePath_ReturnsCorrectPath)
{
    fs::path tempDir = fs::temp_directory_path() / "config_path_test4";
    fs::path projectDir = tempDir / "TestProject";
    fs::path logDir = projectDir / "log";
    
    fs::remove_all(tempDir);
    fs::create_directories(logDir);
    
    std::ofstream(projectDir / "TestProject.json") << "{}";
    std::ofstream(logDir / "TestProject.log") << "";
    
    ConfigFileForTesting config(tempDir.string().c_str(), "TestProject");
    
    std::string expected = tempDir.string() + "/TestProject/TestProject.json";
    EXPECT_EQ(config.configFilePath(), expected);
    
    fs::remove_all(tempDir);
}

/**
 * @test Verify projectPath() returns correct path
 */
TEST_F(ConfigFileTests, ProjectPath_ReturnsCorrectPath)
{
    fs::path tempDir = fs::temp_directory_path() / "config_path_test5";
    fs::path projectDir = tempDir / "TestProject";
    fs::path logDir = projectDir / "log";
    
    fs::remove_all(tempDir);
    fs::create_directories(logDir);
    
    std::ofstream(projectDir / "TestProject.json") << "{}";
    std::ofstream(logDir / "TestProject.log") << "";
    
    ConfigFileForTesting config(tempDir.string().c_str(), "TestProject");
    
    std::string expected = tempDir.string() + "/TestProject/";
    EXPECT_EQ(config.projectPath(), expected);
    
    fs::remove_all(tempDir);
}

/*
 * NOTE: These are pure unit tests focused on validation and error handling.
 * Integration tests that involve file I/O are in ConfigFileIntegrationTests.cpp
 */



