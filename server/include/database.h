#ifndef YOYO_TEMP_DATABASE
#define YOYO_TEMP_DATABASE

#include "packet.h"

#include <vector>

#include "sqlite3.h"

class Database {
public:
    Database();
    void open(void);
    void close(void);
    void append(const yoyotemp_packet_t& pkt);

    // Read-only
    yoyotemp_data_t get_last(void);
    const std::vector<yoyotemp_data_t>& get_today();
    yoyotemp_data_t get_day_max(int64_t day);
    yoyotemp_data_t get_day_min(int64_t day);
    yoyotemp_data_t get_month_max(int64_t day);
    yoyotemp_data_t get_month_min(int64_t day);
    const std::vector<yoyotemp_data_t>& get_monthly_max(int64_t from, int64_t to);
    const std::vector<yoyotemp_data_t>& get_monthly_min(int64_t from, int64_t to);
    const std::vector<yoyotemp_data_t>& get_from_to(int64_t from, int64_t to);
private:
    const std::vector<yoyotemp_data_t>& query(const char *str);
    yoyotemp_data_t get_day_extremum(int64_t day, bool maximum);
    yoyotemp_data_t get_month_extremum(int64_t month, bool maximum);
    const std::vector<yoyotemp_data_t>& get_monthly_extremum(int64_t from, int64_t to, bool maximum);

    sqlite3* db;
    sqlite3_stmt* insertStmt;
    sqlite3_stmt* latestStmt;
    std::vector<yoyotemp_data_t> vec;
};

#endif // YOYO_TEMP_DATABASE