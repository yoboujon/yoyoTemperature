#include "server.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <functional>
#include <stdexcept>

static inline int64_t iso8601_to_epoch(const std::string &date)
{
    std::tm tm{};
    tm.tm_isdst = -1;

    std::istringstream iss(date);
    iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (iss.fail())
        throw std::runtime_error("Invalid date format (expected yyyy-mm-ddThh:mm:ss)");
    return static_cast<int64_t>(std::mktime(&tm));
}

static inline int64_t date_to_epoch(const std::string &date, bool max = false)
{
    std::tm tm{};
    tm.tm_isdst = -1;

    std::istringstream iss(date);
    iss >> std::get_time(&tm, "%d-%m-%Y");
    if (iss.fail())
        throw std::runtime_error("Invalid date format (expected dd-mm-yyyy)");

    tm.tm_hour = max ? 23 : 0;
    tm.tm_min = max ? 59 : 0;
    tm.tm_sec = max ? 60 : 0;
    return static_cast<int64_t>(std::mktime(&tm));
}

static inline int64_t month_to_epoch(const std::string &date, bool max = false)
{
    std::tm tm{};
    tm.tm_isdst = -1;

    std::istringstream iss(date);
    iss >> std::get_time(&tm, "%m-%Y");
    if (iss.fail())
        throw std::runtime_error("Invalid date format (expected mm-yyyy)");

    tm.tm_mday = 1;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;

    if (!max)
        return static_cast<int64_t>(std::mktime(&tm));
    ++tm.tm_mon;
    return static_cast<int64_t>(std::mktime(&tm) - 1);
}

Server::Server(uint16_t p, Database *d)
    : port(p), db(d)
{
}

Server::~Server()
{
    this->close();
}

void Server::open(void)
{
    app.loglevel(crow::LogLevel::Warning);
    app.signal_clear();
    CROW_ROUTE(app, "/")(std::bind(&Server::index, this));
    CROW_ROUTE(app, "/latest")(std::bind(&Server::get_latest, this));
    CROW_ROUTE(app, "/today")(std::bind(&Server::get_today, this));
    CROW_ROUTE(app, "/measurements")(std::bind(&Server::get_from_to, this, std::placeholders::_1));
    CROW_ROUTE(app, "/measurements/day/<string>/<string>")(std::bind(&Server::get_day_extremum, this, std::placeholders::_1, std::placeholders::_2));
    CROW_ROUTE(app, "/measurements/exists/<string>")(std::bind(&Server::get_day_exists, this, std::placeholders::_1));
    CROW_ROUTE(app, "/measurements/day/<string>")(std::bind(&Server::get_day_measurements, this, std::placeholders::_1));
    CROW_ROUTE(app, "/measurements/month/<string>")(std::bind(&Server::get_month_measurements, this, std::placeholders::_1));
    CROW_ROUTE(app, "/measurements/month/<string>/<string>")(std::bind(&Server::get_month_extremum, this, std::placeholders::_1, std::placeholders::_2));
    app.port(port);
}

void Server::close(void)
{
    app.stop();
}

void Server::run(void)
{
    app.run();
}

crow::json::wvalue Server::format_json(bool data)
{
    return crow::json::wvalue{{"state", data}};
}

crow::json::wvalue Server::format_json(const yoyotemp_data_t &data)
{
    const std::time_t t = static_cast<std::time_t>(data.epoch);
    const std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");

    return crow::json::wvalue{
        {"epoch", data.epoch},
        {"date", oss.str()},
        {"temperature", data.temp},
        {"humidity", data.humidity}};
}

crow::json::wvalue Server::format_json(const std::vector<yoyotemp_data_t> &data)
{
    crow::json::wvalue::list list;

    for (const auto &d : data)
    {
        const std::time_t t = d.epoch;
        const std::tm tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");

        crow::json::wvalue item;
        item["epoch"] = d.epoch;
        item["date"] = oss.str();
        item["temperature"] = d.temp;
        item["humidity"] = d.humidity;

        list.emplace_back(std::move(item));
    }

    crow::json::wvalue result;
    result["measurements"] = std::move(list);
    return result;
}

// Pages

crow::response Server::index(void)
{
    try
    {
        const yoyotemp_data_t &data = db->get_last();
        return format_json(data);
    }
    catch (const std::exception &e)
    {
        return crow::response(crow::status::INTERNAL_SERVER_ERROR, e.what());
    }
}

crow::response Server::get_latest(void)
{
    try
    {
        yoyotemp_data_t data = db->get_last();
        return format_json(data);
    }
    catch (const std::exception &e)
    {
        return crow::response(crow::status::INTERNAL_SERVER_ERROR, e.what());
    }
}

crow::response Server::get_today(void)
{
    try
    {
        const std::vector<yoyotemp_data_t> &data = db->get_today();
        return format_json(data);
    }
    catch (const std::exception &e)
    {
        return crow::response(crow::status::INTERNAL_SERVER_ERROR, e.what());
    }
}

crow::response Server::get_from_to(const crow::request &req)
{
    try
    {
        const char *from = req.url_params.get("from"), *to = req.url_params.get("to");
        if (!from || !to)
            return crow::response(400, "API call must be 'measurements?from=&to='");

        int64_t from_epoch = 0, to_epoch = 0;
        if (std::string(from).contains('T') && std::string(to).contains('T'))
            from_epoch = iso8601_to_epoch(from), to_epoch = iso8601_to_epoch(to);
        else
            from_epoch = date_to_epoch(from), to_epoch = date_to_epoch(to, true);

        const std::vector<yoyotemp_data_t> &data = db->get_from_to(from_epoch, to_epoch);
        return format_json(data);
    }
    catch (const std::exception &e)
    {
        return crow::response(crow::status::INTERNAL_SERVER_ERROR, e.what());
    }
}

crow::response Server::get_day_exists(const std::string &day)
{
    const int64_t from = date_to_epoch(day), to = date_to_epoch(day, true);

    try
    {
        const bool data = db->has_from_to(from, to);
        return format_json(data);
    }
    catch (const std::exception &e)
    {
        return crow::response(crow::status::INTERNAL_SERVER_ERROR, e.what());
    }
}

crow::response Server::get_day_measurements(const std::string &day)
{
    const int64_t from = date_to_epoch(day), to = date_to_epoch(day, true);

    try
    {
        const std::vector<yoyotemp_data_t> &data = db->get_from_to(from, to);
        return format_json(data);
    }
    catch (const std::exception &e)
    {
        return crow::response(crow::status::INTERNAL_SERVER_ERROR, e.what());
    }
}

crow::response Server::get_day_extremum(const std::string &day, const std::string &what)
{
    const int64_t day_epoch = date_to_epoch(day);
    std::function<yoyotemp_data_t(int64_t)> fun;

    if (what == "max")
        fun = [this, day_epoch](int64_t day)
        { return this->db->get_day_max(day); };
    else if (what == "min")
        fun = [this, day_epoch](int64_t day)
        { return this->db->get_day_min(day); };
    else
        return crow::response(crow::status::NOT_FOUND);
    try
    {
        yoyotemp_data_t data = fun(day_epoch);
        return format_json(data);
    }
    catch (const std::exception &e)
    {
        return crow::response(crow::status::INTERNAL_SERVER_ERROR, e.what());
    }
}

crow::response Server::get_month_measurements(const std::string &month)
{
    const int64_t from = month_to_epoch(month), to = month_to_epoch(month, true);

    try
    {
        const std::vector<yoyotemp_data_t> &data = db->get_from_to(from, to);
        return format_json(data);
    }
    catch (const std::exception &e)
    {
        return crow::response(crow::status::INTERNAL_SERVER_ERROR, e.what());
    }
}

crow::response Server::get_month_extremum(const std::string &month, const std::string &what)
{
    const int64_t month_epoch = month_to_epoch(month);
    std::function<yoyotemp_data_t(int64_t)> fun;

    if (what == "max")
        fun = [this, month_epoch](int64_t month)
        { return this->db->get_month_max(month); };
    else if (what == "min")
        fun = [this, month_epoch](int64_t month)
        { return this->db->get_month_min(month); };
    else
        return crow::response(crow::status::NOT_FOUND);
    try
    {
        yoyotemp_data_t data = fun(month_epoch);
        return format_json(data);
    }
    catch (const std::exception &e)
    {
        return crow::response(crow::status::INTERNAL_SERVER_ERROR, e.what());
    }
}
