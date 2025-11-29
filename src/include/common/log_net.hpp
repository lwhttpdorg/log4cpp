#pragma once

#include <string>

#if defined(_WIN32)
#include <WinSock2.h>
#include <windows.h>
#endif

#ifdef __linux

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#endif

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#include "log_utils.hpp"

#include <nlohmann/json.hpp>

namespace log4cpp::common {
#ifdef _WIN32
    constexpr SOCKET INVALID_FD = INVALID_SOCKET;
    typedef SOCKET socket_fd;

    inline void close_socket(socket_fd fd) {
        closesocket(fd);
    }
#endif
#if defined(__linux__)
    constexpr int INVALID_FD = -1;
    typedef int socket_fd;

    inline void close_socket(socket_fd fd) {
        close(fd);
    }

#endif

#ifdef _WIN32
    // Windows socket initialization
    class socket_init {
    public:
        socket_init() {
            WSADATA wsa_data{};
            (void)WSAStartup(MAKEWORD(2, 2), &wsa_data);
        }

        ~socket_init() {
            WSACleanup();
        }
    };
#endif

    enum class net_family { NET_IPv4, NET_IPv6 };
    enum class prefer_stack { IPv4, IPv6, AUTO };

    void to_string(prefer_stack prefer, std::string &str);

    void from_string(const std::string &str, prefer_stack &prefer);

    class host_resolve_exception: public std::runtime_error {
    public:
        explicit host_resolve_exception(const std::string &msg) : std::runtime_error(msg) {
        }
    };

    class net_addr {
    public:
        union addr_u {
            unsigned int addr4;
            unsigned int addr6[4];
        };

        net_family family{net_family::NET_IPv4};
        addr_u ip{};

        net_addr();

        explicit net_addr(const char *addr);

        explicit net_addr(const std::string &addr);

        bool operator==(const net_addr &rhs) const;

        bool operator!=(const net_addr &rhs) const;

        [[nodiscard]] std::string to_string() const;

        static net_addr resolve(const std::string &host, prefer_stack prefer = prefer_stack::AUTO);
    };

    void to_json(nlohmann::json &j, const net_addr &addr);

    void from_json(const nlohmann::json &j, net_addr &addr);

    class sock_addr {
    public:
        net_addr addr{};
        unsigned short port{0};

        sock_addr();

        explicit sock_addr(const char *ip, unsigned short p);

        explicit sock_addr(const std::string &ip, unsigned short p);

        bool operator==(const sock_addr &rhs) const;

        bool operator!=(const sock_addr &rhs) const;

        [[nodiscard]] std::string to_string() const {
            return addr.to_string() + "@" + std::to_string(port);
        }
    };
}

namespace std {
    template<>
    struct hash<log4cpp::common::net_addr> {
        std::size_t operator()(const log4cpp::common::net_addr &addr) const noexcept {
            std::size_t h = 0;
            if (addr.family == log4cpp::common::net_family::NET_IPv4) {
                h = std::hash<unsigned int>{}(addr.ip.addr4);
            }
            else {
                for (const auto x: addr.ip.addr6) {
                    h ^= std::hash<unsigned int>{}(x);
                }
            }
            return h;
        }
    };

    template<>
    struct hash<log4cpp::common::sock_addr> {
        std::size_t operator()(const log4cpp::common::sock_addr &saddr) const noexcept {
            std::size_t h = std::hash<log4cpp::common::net_addr>{}(saddr.addr);
            h ^= std::hash<unsigned short>{}(saddr.port);
            return h;
        }
    };
}
