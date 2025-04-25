#include "sqlapplet.h"
#include "gtest/gtest.h"

TEST(AppletTests, WrongAppletPath)
{
    // Initialize project applet path
    SQLApplet::InitPathToApplets("XX:/No Path to Applets");
    SQLApplet applet("applet");
    try {
        applet.parse();
    } catch(const SQLAppletException & e) {
        // and this tests that it has the correct message
        EXPECT_STREQ(APPLET_ERR_WRONG_PATH.c_str(), e.what());
    } catch (...) {
        FAIL() << "Expected a different exception type.";
    }
}

TEST(AppletTests, LoadConfigAndInitApplets)
{
       //SQLApplet::InitPathToApplets(config->appletPath().c_str());
}

TEST(AppletTests, VoidTest)
{
   ASSERT_TRUE(true);
}
