#pragma once

#include <string>

namespace log4cpp::net
{
#if defined(_MSC_VER) || defined(__WIN32)
    constexpr SOCKET INVALID_SOCKET = INVALID_SOCKET;
#endif
#if defined(__linux__)
    constexpr int INVALID_SOCKET = -1;
#endif

    enum class net_family
    {
        NET_IPv4,
        NET_IPv6
    };

    class net_addr
    {
    public:
        union addr_u
        {
            unsigned int addr4;
            unsigned int addr6[4];
        };

        net_family family{net_family::NET_IPv4};
        addr_u ip{};

        friend std::string to_string(const net_addr &addr);

        friend net_addr from_string(const std::string &s);
    };

    std::string to_string(const net_addr &addr);

    net_addr from_string(const std::string &s);
}

