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
    const std::vector<yoyotemp_maxmin_t>& get_daily(int64_t from, int64_t to);
    yoyotemp_data_t get_month_max(int64_t day);
    yoyotemp_data_t get_month_min(int64_t day);
    const std::vector<yoyotemp_data_t>& get_from_to(int64_t from, int64_t to);
    bool has_from_to(int64_t from, int64_t to);
private:
    // Query SQL
    const std::vector<yoyotemp_data_t>& query(const char *str);
    bool query_bool(const char *str);
    const std::vector<yoyotemp_maxmin_t>& query_maxmin(const char *str);

    // Max/Min
    yoyotemp_data_t get_day_extremum(int64_t day, bool maximum);
    yoyotemp_data_t get_month_extremum(int64_t month, bool maximum);

    sqlite3* db;
    sqlite3_stmt* insertStmt;
    sqlite3_stmt* latestStmt;
    std::vector<yoyotemp_data_t> vec;
    std::vector<yoyotemp_maxmin_t> vec_maxmin;
};

#endif // YOYO_TEMP_DATABASE