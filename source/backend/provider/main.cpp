#include <iostream>

#include <nlohmann/json.hpp>
#include <SQLAPI.h>
#include <easylogging++.h>

#include "configfile.h"
#include "sqlapplet.h"
#include "include_backend_util.h"

#include "sentry.h"

//#include <httplib.h>
//using namespace std;
//using namespace httplib;
//using json = nlohmann::json;

ConfigFile qGlobalConfig;

INITIALIZE_EASYLOGGINGPP

static sentry_value_t on_crash_callback(const sentry_ucontext_t *uctx, sentry_value_t event, void *closure)
{
    (void)uctx;
    (void)closure;

    std::cout<<"Crash happened"<<std::endl;
    return event;
}
static sentry_value_t
before_send_callback(sentry_value_t event, void *hint, void *closure)
{
    (void)hint;
    (void)closure;

    // make our mark on the event
    sentry_value_set_by_key(
        event, "adapted_by", sentry_value_new_string("before_send"));

    // tell the backend to proceed with the event
    return event;
}
static void *invalid_mem = (void *)0xFFFFFFFFFFFFFF9B; // -100 for memset
static void trigger_crash()
{
    memset((char *)invalid_mem, 1, 100);
}

int main()
{
    //json j;
    //j["pi"] = 3.141;
    //j["happy"] = true;
    //j["name"] = "Niels";
    //j["nothing"] = nullptr;
    //j["answer"]["everything"] = 42;
    //j["list"] = {1, 0, 2};
    //j["object"] = {{"currency", "USD"}, {"value", 42.99}};

    //auto j2 =
    //    R"(
    //        {
    //            "happy": true,
    //            "pi": 3.141
    //        }
    //    )"_json;

    //ifstream i("file.json");
    //json j;
    //i >> j;

    sentry_options_t *options = sentry_options_new();

    //if (has_arg(argc, argv, "disable-backend")) {
       //sentry_options_set_backend(options, NULL);
    //}

    // this is an example. for real usage, make sure to set this explicitly to
    // an app specific cache location.
    sentry_options_set_database_path(options, ".sentry-native");
    sentry_options_set_debug(options, 1);
    sentry_options_set_handler_path(options,
    "C:/projects/MediCon/source/backend/provider/build/x86-64/Debug/build/Debug/provider.exe");

    sentry_options_set_before_send(options, before_send_callback, NULL);
    sentry_options_set_system_crash_reporter_enabled(options, true);
    sentry_options_set_on_crash(options, on_crash_callback, NULL);

    //sentry_options_set_auto_session_tracking(options, false);
    //sentry_options_set_symbolize_stacktraces(options, true);

    //sentry_options_set_environment(options, "development");
    //sentry_options_set_dsn(options, "http://127.0.0.1:80");



    sentry_init(options);


    //sentry_value_t event = sentry_value_new_message_event(SENTRY_LEVEL_INFO, "my-logger", "Hello World!");
    //sentry_event_value_add_stacktrace(event, NULL, 0);
    //sentry_capture_event(event);
    sentry_value_t event = sentry_value_new_event();
    sentry_value_set_by_key(event, "my-massage", sentry_value_new_string("Hello WWWWWWW!"));
    sentry_capture_event(event);

    //try {
        trigger_crash();
    //} catch(...) {

    //}

    sentry_close();
    //============================================================
     // Initialize global config object
    qGlobalConfig.setProjectPath(ALL_PROJECT_APPDATA_PATH, PROJECT_NAME);
    if(!qGlobalConfig.load()) {
        return -1;
    }

    // Initialize logger with global settings
    el::Configurations qGlobalLog;
    qGlobalLog.setGlobally(el::ConfigurationType::Format, "%user:%fbase:%line:%datetime:%level:%msg:");
    qGlobalLog.setGlobally(el::ConfigurationType::Filename, qGlobalConfig.logFilePath());
    qGlobalLog.set(el::Level::Global, el::ConfigurationType::ToFile, "true");
    qGlobalLog.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "false");
    el::Loggers::setDefaultConfigurations(qGlobalLog, true);

    TypeToStringFormatter formatter;

    // Initialize project applet path
    SQLApplet::InitPath(qGlobalConfig.appletPath().c_str());

    //============================================
    SQLApplet applet("illnessgroup_name_update.xml",
                     {{"NAME", "Vakho"},
                      {"R_ILLNESS_GROUP_UID", "10"},
                      {"R_LANGUAGE_UID", "20"}
                     });
    applet.parse();
    LOG(INFO) << applet.sql();

    SQLApplet applet2("illnessgroup_select.xml");
    applet2.parse();
    LOG(INFO) << applet2.sql();
    //============================================

    SAString  db_host = qGlobalConfig["host"].c_str();
    SAString  db_user = qGlobalConfig["user"].c_str();
    SAString  db_pass = qGlobalConfig["pass"].c_str();
    SAConnection con;
    try
    {
        con.setClient( SA_PostgreSQL_Client );
        con.Connect(db_host, db_user, db_pass);
        SACommand select(&con, _TSA(""));

        con.Disconnect();

    } catch(SAException & x) {
        // SAConnection::Rollback()
        // can also throw an exception
        // (if a network error for example),
        // we will be ready
        try
        {
            // on error rollback changes
            con.Rollback();
        } catch(SAException &) {

        }
        // print error message
        //LOG(ERROR) << "DBConnection: " << x.ErrText().GetMultiByteChars();
    }

    return 0;
    //Server svr;

    //svr.Get("/hi", [](const Request & /*req*/, Response &res) {
    //    res.set_content("Hello World!", "text/plain");
    //});

    //svr.listen("127.0.0.1", 8085);
}
