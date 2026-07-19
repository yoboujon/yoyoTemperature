#ifndef YOYO_TEMP_THERMOMETHER
#define YOYO_TEMP_THERMOMETHER

#include "packet.h"
#include "database.h"

#include <cstdint>
#include <atomic>

class Thermometer {
public:
    Thermometer(uint16_t port, Database* db);
    ~Thermometer();
    void open(void);
    void close(void);
    void run(void);
private:
    uint16_t port;
    int sock;
    std::atomic<bool> stop;
    bool stopped;
    yoyotemp_packet_t temp_packet;
    Database* db;
};

#endif // YOYO_TEMP_THERMOMETHER