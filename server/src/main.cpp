#include <print>
#include <thread>
#include <csignal>
#include <atomic>

#include "database.h"
#include "thermometer.h"
#include "server.h"
#include "constants.h"

static volatile int keepRunning = 1;

void signal_handler(int)
{
    keepRunning = 0;
}

int main(void)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::setvbuf(stderr, nullptr, _IONBF, 0);
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    Database db;
    Thermometer t(UDP_PORT, &db);
    Server s(API_PORT, &db);

    try {
        db.open();
        t.open();
        s.open();
    } catch (const std::exception& err) {
        std::println("Error: {}", err.what());
        return -1;
    }

    std::thread th_therm(&Thermometer::run, &t);
    std::thread th_server(&Server::run, &s);
    std::println("Started Serving !");

    while (keepRunning)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::println("Shutting down...");
    t.close();
    s.close();
    db.close();

    th_therm.join();
    th_server.join();
    return 0;
}