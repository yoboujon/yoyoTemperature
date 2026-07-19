#include "thermometer.h"

#include <stdexcept>
#include <format>
#include <thread>
#include <chrono>
#include <cstring>
#include <print>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#define BUF_SIZE 1024

static inline int set_recv_timeout(int sock, int timeout_ms)
{
#ifdef _WIN32
    DWORD tv = timeout_ms;
    return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
#else
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
}

Thermometer::Thermometer(uint16_t p, Database* d)
    : port(p), sock(INVALID_SOCKET), stop(false), stopped(true), db(d)
{
}

Thermometer::~Thermometer()
{
    this->close();
}

void Thermometer::open(void)
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed\n");
        return 1;
    }
#endif

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
    {
        throw std::runtime_error(std::format("socket creation failed ({})", strerror(errno)));
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        ::closesocket(sock);
        throw std::runtime_error(std::format("bind failed ({})", strerror(errno)));
    }
}

void Thermometer::close(void)
{
    stop.store(true);
    while (!stopped)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    ::closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif
}

void Thermometer::run(void)
{
    char buffer[BUF_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    set_recv_timeout(sock, 2000);
    stop.store(false);
    stopped = false;

    while (!stop.load())
    {
        memset(buffer, 0, BUF_SIZE);
        int recv_len = recvfrom(sock, buffer, BUF_SIZE - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (recv_len == sizeof(yoyotemp_packet_t))
        {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
            std::memcpy(&temp_packet, buffer, recv_len);
            db->append(temp_packet);
            
            const uint16_t humidity = temp_packet.humidity;
            const float temp = temp_packet.temp;
            std::println("[Received from {}:{}]\ttemp: {}°C\t humidity: {:.1f}%", client_ip, ntohs(client_addr.sin_port), temp, humidity/10.0f);
        }
    }
    stopped = true;
}

