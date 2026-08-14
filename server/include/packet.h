#ifndef YOYO_TEMP_PACKET
#define YOYO_TEMP_PACKET

#include <cstdint>

typedef struct __attribute__((packed)) yoyotemp_packet {
    uint16_t humidity;
    float temp;
} yoyotemp_packet_t;

typedef struct yoyotemp_data {
    int64_t epoch;
    float humidity;
    float temp;
} yoyotemp_data_t;

typedef struct yoyotemp_maxmin {
    int64_t epoch;
    float max;
    float min;
} yoyotemp_maxmin_t;

#endif // YOYO_TEMP_PACKET