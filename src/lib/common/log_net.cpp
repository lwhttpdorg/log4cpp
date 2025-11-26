#include <stdexcept>

#if defined(__linux__)

#include <arpa/inet.h>
#include <netinet/in.h>

#endif

#if defined(_WIN32)
#include <WS2tcpip.h>
#include <ws2ipdef.h>
#endif

#include "common/log_net.hpp"

namespace log4cpp::common {
#ifdef _WIN32
    // Windows socket initialization
    static socket_init net_init{};
#endif

    net_addr::net_addr() {
        this->family = net_family::NET_IPv4;
        this->ip.addr4 = 0;
    }

    net_addr::net_addr(const char *addr) {
        in_addr addr4{};
        in6_addr addr6{};
        if (0 < inet_pton(AF_INET, addr, &addr4)) {
            this->family = net_family::NET_IPv4;
            this->ip.addr4 = addr4.s_addr;
        }
        else if (0 < inet_pton(AF_INET6, addr, &addr6)) {
            this->family = net_family::NET_IPv6;
#if defined(__linux__)
            this->ip.addr6[0] = addr6.s6_addr32[0];
            this->ip.addr6[1] = addr6.s6_addr32[1];
            this->ip.addr6[2] = addr6.s6_addr32[2];
            this->ip.addr6[3] = addr6.s6_addr32[3];
#endif
#if defined(_WIN32)
            this->ip.addr6[0] = addr6.u.Word[0];
            this->ip.addr6[1] = addr6.u.Word[1];
            this->ip.addr6[2] = addr6.u.Word[2];
            this->ip.addr6[3] = addr6.u.Word[3];
#endif
        }
        else {
            throw std::invalid_argument("Invalid IP address string: " + std::string{addr});
        }
    }

    net_addr::net_addr(const std::string &addr) : net_addr(addr.c_str()) {
    }

    bool net_addr::operator==(const net_addr &rhs) const {
        if (family != rhs.family) {
            return false;
        }
        if (family == net_family::NET_IPv4) {
            return ip.addr4 == rhs.ip.addr4;
        }
        return ip.addr6[0] == rhs.ip.addr6[0] && ip.addr6[1] == rhs.ip.addr6[1] && ip.addr6[2] == rhs.ip.addr6[2]
               && ip.addr6[3] == rhs.ip.addr6[3];
    }

    bool net_addr::operator!=(const net_addr &rhs) const {
        return !(rhs == *this);
    }

    std::string net_addr::to_string() const {
        std::string s;
        if (this->family == net_family::NET_IPv4) {
            char buf[INET_ADDRSTRLEN];
            const unsigned char a = this->ip.addr4 >> 24 & 0xff;
            const unsigned char b = this->ip.addr4 >> 16 & 0xff;
            const unsigned char c = this->ip.addr4 >> 8 & 0xff;
            const unsigned char d = this->ip.addr4 & 0xff;
            snprintf(buf, sizeof(buf), "%u.%u.%u.%u", a, b, c, d);
            s = std::string{buf};
        }
        else if (this->family == net_family::NET_IPv6) {
            char buf[INET6_ADDRSTRLEN];
            int len = 0;
            for (const auto x: this->ip.addr6) {
                const unsigned short a = x >> 16;
                const unsigned short b = x & 0xffff;
                if (const int l = snprintf(buf + len, sizeof(buf) - len, "%x%x:", a, b); l > 0) {
                    len += l;
                }
                else {
                    break;
                }
            }
            buf[len - 1] = '\0';
            s = std::string{buf};
        }
        else {
            throw std::invalid_argument("Invalid addr family \"" + std::to_string(static_cast<int>(this->family))
                                        + "\"");
        }
        return s;
    }

    class addrinfo_guard {
    public:
        explicit addrinfo_guard(addrinfo *res) : res_(res) {
        }
        ~addrinfo_guard() {
            if (res_) {
                freeaddrinfo(res_);
            }
        }
        addrinfo_guard(const addrinfo_guard &) = delete;
        addrinfo_guard(addrinfo_guard &&) = delete;
        addrinfo_guard &operator=(const addrinfo_guard &) = delete;
        addrinfo_guard &operator=(addrinfo_guard &&) = delete;

    private:
        addrinfo *res_;
    };

    net_addr net_addr::resolve(const std::string &host, prefer_stack prefer) {
        // If host is an IP address, return it directly; otherwise, perform DNS resolution
        // Check if host is an IP address
        try {
            net_addr addr(host);
            return addr;
        }
        catch (const std::invalid_argument &) {
            // continue to DNS resolution
        }
        // Not an IP address, perform DNS resolution
        addrinfo hints{};
        if (prefer == prefer_stack::IPv4) {
            hints.ai_family = AF_INET;
        }
        else if (prefer == prefer_stack::IPv6) {
            hints.ai_family = AF_INET6;
        }
        else {
            hints.ai_family = AF_UNSPEC;
        }
        // TCP or UDP
        hints.ai_socktype = SOCK_STREAM;

        addrinfo *res = nullptr;
        int ret = getaddrinfo(host.c_str(), nullptr, &hints, &res);
        if (ret != 0) {
            throw host_resolve_exception("Failed to resolve host: " + host + ", error: " + gai_strerror(ret));
        }
        addrinfo_guard res_guard(res);

        net_addr addr;
        bool resolved = false;
        for (const addrinfo *p = res; p != nullptr; p = p->ai_next) {
            if (p->ai_family == AF_INET) {
                const sockaddr_in *addr4 = reinterpret_cast<sockaddr_in *>(p->ai_addr);
                addr.family = net_family::NET_IPv4;
                addr.ip.addr4 = addr4->sin_addr.s_addr;
                resolved = true;
                break;
            }
            if (p->ai_family == AF_INET6) {
                const sockaddr_in6 *addr6 = reinterpret_cast<sockaddr_in6 *>(p->ai_addr);
                addr.family = net_family::NET_IPv6;
                std::memcpy(addr.ip.addr6, addr6->sin6_addr.s6_addr, sizeof(addr.ip.addr6));
                resolved = true;
                break;
            }
        }
        if (!resolved) {
            throw host_resolve_exception("No valid address found for host: " + host);
        }
        return addr;
    }

    void to_json(nlohmann::json &j, const net_addr &addr) {
        j = addr.to_string();
    }

    void from_json(const nlohmann::json &j, net_addr &addr) {
        std::string s = j.get<std::string>();
        addr = net_addr(s);
    }

    sock_addr::sock_addr() {
        addr = net_addr{};
        port = 0;
    }

    sock_addr::sock_addr(const char *ip, unsigned short p) {
        this->addr = net_addr(ip);
        this->port = p;
    }

    sock_addr::sock_addr(const std::string &ip, unsigned short p) : sock_addr(ip.c_str(), p) {
    }

    bool sock_addr::operator==(const sock_addr &rhs) const {
        return addr == rhs.addr && port == rhs.port;
    }

    bool sock_addr::operator!=(const sock_addr &rhs) const {
        return !(rhs == *this);
    }
}
