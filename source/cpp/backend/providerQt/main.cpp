#include <QCoreApplication>
#include <httplib.h>
#include <nlohmann/json.hpp>

using namespace httplib;

int main(int argc, char *argv[])
{
    Server svr;

    svr.Get("/hi", [](const Request & /*req*/, Response &res) {
        res.set_content("Hello World!", "text/plain");
    });

    svr.listen("127.0.0.1", 8085);
}
