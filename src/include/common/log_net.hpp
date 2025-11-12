#pragma once

#include <string>

#if defined(_WIN32)
#include <WinSock2.h>
#include <windows.h>
#endif

#ifdef __linux

#include <unistd.h>

#endif

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

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
            WSAStartup(MAKEWORD(2, 2), &wsa_data);
        }

        ~socket_init() {
            WSACleanup();
        }
    };
#endif

    enum class net_family { NET_IPv4, NET_IPv6 };

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

        friend void to_json(nlohmann::json &j, const net_addr &addr);

        friend void from_json(const nlohmann::json &j, net_addr &addr);
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
