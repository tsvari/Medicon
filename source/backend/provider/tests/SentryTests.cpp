#include "gtest/gtest.h"
//#include "sentry.h"


//static sentry_value_t on_crash_callback(const sentry_ucontext_t *uctx, sentry_value_t event, void *closure)
//{
//    (void)uctx;
//    (void)closure;
//
//    std::cout<<"Crash happened"<<std::endl;
//    return event;
//}
//static sentry_value_t
//before_send_callback(sentry_value_t event, void *hint, void *closure)
//{
//    (void)hint;
//    (void)closure;
//
//    // make our mark on the event
//    sentry_value_set_by_key(
//        event, "adapted_by", sentry_value_new_string("before_send"));
//
//    // tell the backend to proceed with the event
//    return event;
//}
//static void *invalid_mem = (void *)0xFFFFFFFFFFFFFF9B; // -100 for memset
//static void trigger_crash()
//{
//    memset((char *)invalid_mem, 1, 100);
//}

TEST(ConfigFileTests, SentryTests)
{
    // main()
        //sentry_options_t *options = sentry_options_new();

        //if (has_arg(argc, argv, "disable-backend")) {
        //sentry_options_set_backend(options, NULL);
        //}

        // this is an example. for real usage, make sure to set this explicitly to
        // an app specific cache location.
        //sentry_options_set_database_path(options, ".sentry-native");
        //sentry_options_set_debug(options, 1);
        //sentry_options_set_handler_path(options,
        //                                "C:/projects/MediCon/source/backend/provider/build/x86-64/Debug/build/Debug/provider.exe");

        //sentry_options_set_before_send(options, before_send_callback, NULL);
        //sentry_options_set_system_crash_reporter_enabled(options, true);
        //sentry_options_set_on_crash(options, on_crash_callback, NULL);

        //sentry_options_set_auto_session_tracking(options, false);
        //sentry_options_set_symbolize_stacktraces(options, true);

        //sentry_options_set_environment(options, "development");
        //sentry_options_set_dsn(options, "http://127.0.0.1:80");



        //sentry_init(options);


        //sentry_value_t event = sentry_value_new_message_event(SENTRY_LEVEL_INFO, "my-logger", "Hello World!");
        //sentry_event_value_add_stacktrace(event, NULL, 0);
        //sentry_capture_event(event);
        //sentry_value_t event = sentry_value_new_event();
        //sentry_value_set_by_key(event, "my-massage", sentry_value_new_string("Hello WWWWWWW!"));
        //sentry_capture_event(event);

        ////try {
        //trigger_crash();
        ////} catch(...) {

        ////}

        //sentry_close();
        //============================================================
}
