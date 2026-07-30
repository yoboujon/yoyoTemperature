#ifndef YOYO_TEMP_SERVER
#define YOYO_TEMP_SERVER

#include "database.h"

#include <vector>

#include "crow.h"

struct ServerDate {
    int day;
    int month;
    int year;
};

class Server {
public:
    Server(uint16_t port, Database* db);
    ~Server();
    void open(void);
    void close(void);
    void run(void);
private:
    crow::json::wvalue format_json(const yoyotemp_data_t& data);
    crow::json::wvalue format_json(const std::vector<yoyotemp_data_t>& data);
    crow::json::wvalue format_json(bool data);

    // Crow pages
    crow::response index(void);
    crow::response get_latest(void);
    crow::response get_today(void);
    crow::response get_from_to(const crow::request& req);
    crow::response get_day_measurements(const std::string& day);
    crow::response get_day_exists(const std::string &day);
    crow::response get_day_extremum(const std::string& day, const std::string& what);
    crow::response get_month_measurements(const std::string& month);
    crow::response get_month_extremum(const std::string& month, const std::string& what);

    crow::SimpleApp app;
    uint16_t port;
    Database* db;
};

#endif // YOYO_TEMP_SERVER