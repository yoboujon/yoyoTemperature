#include "database.h"

#include <stdexcept>
#include <chrono>

static inline std::time_t get_actual_day(void)
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    return timegm(&tm);
}

static inline int64_t get_midnight(int64_t timestamp)
{
    std::time_t t = static_cast<std::time_t>(timestamp);

    std::tm tm = *std::localtime(&t);
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;

    return timegm(&tm);
}

static inline int64_t get_utc_offset_seconds(int64_t epoch_seconds)
{
    using namespace std::chrono;
    const auto tp = sys_seconds{seconds{epoch_seconds}};
    const auto* tz = current_zone();
    const auto info = tz->get_info(tp);

    return info.offset.count();
}

Database::Database()
{
}

void Database::open(void)
{
    sqlite3_open("temperature.db", &db);
    const char *sql = R"(
CREATE TABLE IF NOT EXISTS measurements (
    timestamp INTEGER PRIMARY KEY,
    temperature REAL NOT NULL,
    humidity REAL NOT NULL
);
    )";
    sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
    sqlite3_prepare_v2(db, "INSERT INTO measurements VALUES (?, ?, ?);", -1, &insertStmt, nullptr);
}

void Database::close(void)
{
    sqlite3_close(db);
}

void Database::append(const yoyotemp_packet_t &pkt)
{
    const int64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    const float humidity = pkt.humidity / 10.0f;
    const float temp = pkt.temp;

    sqlite3_bind_int64(insertStmt, 1, now);
    sqlite3_bind_double(insertStmt, 2, temp);
    sqlite3_bind_double(insertStmt, 3, humidity);

    sqlite3_step(insertStmt);
    sqlite3_reset(insertStmt);
}

const std::vector<yoyotemp_data_t> &Database::query(const char *str)
{
    vec.clear();
    int rc = sqlite3_prepare_v2(db, str, -1, &latestStmt, nullptr);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(sqlite3_errmsg(db));
    }
    sqlite3_reset(latestStmt);

    while ((rc = sqlite3_step(latestStmt)) == SQLITE_ROW)
    {
        const int64_t timestamp = sqlite3_column_int64(latestStmt, 0);
        const float temp = static_cast<float>(sqlite3_column_double(latestStmt, 1));
        const float humidity = static_cast<float>(sqlite3_column_int(latestStmt, 2));
        vec.push_back({timestamp, humidity, temp});
    }
    if (rc != SQLITE_DONE)
    {
        sqlite3_finalize(latestStmt);
        throw std::runtime_error(std::format("Query failed: {}", sqlite3_errmsg(db)));
    }

    sqlite3_finalize(latestStmt);
    return vec;
}

bool Database::query_bool(const char *str)
{
    int rc = sqlite3_prepare_v2(db, str, -1, &latestStmt, nullptr);
    int ret = 0;

    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(sqlite3_errmsg(db));
    }
    sqlite3_reset(latestStmt);

    while ((rc = sqlite3_step(latestStmt)) == SQLITE_ROW)
    {
        ret = sqlite3_column_int(latestStmt, 0);
    }
    if (rc != SQLITE_DONE)
    {
        sqlite3_finalize(latestStmt);
        throw std::runtime_error(std::format("Query failed: {}", sqlite3_errmsg(db)));
    }

    sqlite3_finalize(latestStmt);
    return static_cast<bool>(ret);
}

const std::vector<yoyotemp_maxmin_t> &Database::query_maxmin(const char *str)
{
    vec_maxmin.clear();
    int rc = sqlite3_prepare_v2(db, str, -1, &latestStmt, nullptr);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(sqlite3_errmsg(db));
    }
    sqlite3_reset(latestStmt);

    while ((rc = sqlite3_step(latestStmt)) == SQLITE_ROW)
    {
        const float max = static_cast<float>(sqlite3_column_double(latestStmt, 1));
        const int64_t max_epoch = sqlite3_column_int64(latestStmt, 2);
        const float min = static_cast<float>(sqlite3_column_double(latestStmt, 3));
        const int64_t min_epoch = sqlite3_column_int64(latestStmt, 4);
        vec_maxmin.push_back({max, max_epoch, min, min_epoch});
    }
    if (rc != SQLITE_DONE)
    {
        sqlite3_finalize(latestStmt);
        throw std::runtime_error(std::format("Query failed: {}", sqlite3_errmsg(db)));
    }

    sqlite3_finalize(latestStmt);
    return vec_maxmin;
}

yoyotemp_data_t Database::get_last(void)
{
    const char *sql = R"(
        SELECT timestamp,
            temperature,
            humidity
        FROM measurements
        ORDER BY timestamp DESC
        LIMIT 1;
    )";

    const std::vector<yoyotemp_data_t> &data = this->query(sql);
    if (data.empty())
        throw std::runtime_error("get_last: Empty");
    return data.front();
}

const std::vector<yoyotemp_data_t> &Database::get_today(void)
{
    const std::string sql = std::format(R"(
        SELECT timestamp,
               temperature,
               humidity
        FROM measurements
        WHERE timestamp >= {}
        ORDER BY timestamp;
    )",
                                        get_actual_day());

    const std::vector<yoyotemp_data_t> &data = this->query(sql.c_str());
    if (data.empty())
        throw std::runtime_error("get_today: Empty");
    return data;
}

yoyotemp_data_t Database::get_day_extremum(int64_t day, bool maximum)
{
    day = get_midnight(day);
    const std::string sql = std::format(R"(
        SELECT timestamp,
               temperature,
               humidity
        FROM measurements
        WHERE timestamp >= {}
          AND timestamp < {}
        ORDER BY temperature {}
        LIMIT 1;
    )",
                                        day,
                                        day + 24 * 60 * 60,
                                        maximum ? "DESC" : "ASC");

    const std::vector<yoyotemp_data_t> &data = this->query(sql.c_str());
    if (data.empty())
        throw std::runtime_error(std::format("get_day_{}: Empty", maximum ? "max" : "min"));
    return data.front();
}

yoyotemp_data_t Database::get_day_max(int64_t day)
{
    return get_day_extremum(day, true);
}

yoyotemp_data_t Database::get_day_min(int64_t day)
{
    return get_day_extremum(day, false);
}

const std::vector<yoyotemp_maxmin_t> &Database::get_daily(int64_t from, int64_t to)
{
    const int64_t utc_offset = get_utc_offset_seconds(from);
    const auto sql = std::format(R"(
WITH ranked AS (
    SELECT
        timestamp,
        temperature,
        date(timestamp + {}, 'unixepoch') AS day,
        ROW_NUMBER() OVER (
            PARTITION BY date(timestamp + {}, 'unixepoch')
            ORDER BY temperature DESC
        ) AS max_rank,
        ROW_NUMBER() OVER (
            PARTITION BY date(timestamp + {}, 'unixepoch')
            ORDER BY temperature ASC
        ) AS min_rank
    FROM measurements
    WHERE timestamp BETWEEN {} AND {}
)
SELECT
    day,
    MAX(CASE WHEN max_rank = 1 THEN temperature END) AS max_temperature,
    MAX(CASE WHEN max_rank = 1 THEN timestamp END)   AS max_epoch,
    MAX(CASE WHEN min_rank = 1 THEN temperature END) AS min_temperature,
    MAX(CASE WHEN min_rank = 1 THEN timestamp END)   AS min_epoch
FROM ranked
GROUP BY day
ORDER BY day;
    )",
                                 utc_offset, utc_offset, utc_offset, from, to);
    const auto &data = this->query_maxmin(sql.c_str());

    if (data.empty())
        throw std::runtime_error("get_daily: Empty");
    return data;
}

yoyotemp_data_t Database::get_month_extremum(int64_t month, bool maximum)
{
    std::time_t t = static_cast<std::time_t>(month);
    std::tm tm = *std::localtime(&t);
    ++tm.tm_mon;
    const int64_t next_month = timegm(&tm);

    const auto sql = std::format(R"(
        SELECT timestamp,
               temperature,
               humidity
        FROM measurements
        WHERE timestamp >= {}
          AND timestamp < {}
        ORDER BY temperature {}
        LIMIT 1;
    )",
                                 month,
                                 next_month,
                                 maximum ? "DESC" : "ASC");

    const auto &data = query(sql.c_str());

    if (data.empty())
        throw std::runtime_error(
            std::format("get_month_{}: Empty", maximum ? "max" : "min"));

    return data.front();
}

yoyotemp_data_t Database::get_month_max(int64_t month)
{
    return get_month_extremum(month, true);
}

yoyotemp_data_t Database::get_month_min(int64_t month)
{
    return get_month_extremum(month, false);
}

const std::vector<yoyotemp_data_t> &Database::get_from_to(int64_t from, int64_t to)
{
    const std::string sql = std::format(R"(
        SELECT timestamp,
               temperature,
               humidity
        FROM measurements
        WHERE timestamp >= {}
          AND timestamp <= {}
        ORDER BY timestamp ASC;
    )",
                                        from, to);

    const std::vector<yoyotemp_data_t> &data = this->query(sql.c_str());
    if (data.empty())
        throw std::runtime_error("get_from_to: Empty");
    return data;
}

bool Database::has_from_to(int64_t from, int64_t to)
{
    const std::string sql = std::format(R"(
        SELECT EXISTS(
            SELECT 1
            FROM measurements
            WHERE timestamp >= {}
              AND timestamp <= {}
        );
    )",
                                        from, to);

    return this->query_bool(sql.c_str());
}
