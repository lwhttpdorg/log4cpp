#include <stdexcept>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "log_net.h"

using namespace log4cpp::net;

std::string log4cpp::net::to_string(const net_addr &addr)
{
    std::string s;
    if (addr.family == net_family::NET_IPv4)
    {
        char buf[INET_ADDRSTRLEN];
        unsigned char a, b, c, d;
        a = (addr.ip.addr4 >> 24) & 0xff;
        b = (addr.ip.addr4 >> 16) & 0xff;
        c = (addr.ip.addr4 >> 8) & 0xff;
        d = addr.ip.addr4 & 0xff;
        snprintf(buf, sizeof(buf), "%u.%u.%u.%u", a, b, c, d);
        s = std::string{buf};
    }
    else if (addr.family == net_family::NET_IPv6)
    {
        char buf[INET6_ADDRSTRLEN];
        int len = 0;
        for (auto x: addr.ip.addr6)
        {
            unsigned short a = (x >> 16), b = x & 0xffff;
            int l = snprintf(buf + len, sizeof(buf) - len, "%x%x:", a, b);
            if (l > 0)
            {
                len += l;
            }
            else
            {
                break;
            }
        }
        buf[len - 1] = '\0';
        s = std::string{buf};
    }
    else
    {
        throw std::invalid_argument("Invalid addr family \"" + std::to_string(static_cast<int> (addr.family)) + "\"");
    }
    return s;
}

net_addr log4cpp::net::from_string(const std::string &s)
{
    net_addr addr{};

    in_addr addr4{};
    in6_addr addr6{};
    if (0 < inet_pton(AF_INET, s.c_str(), &addr4))
    {
        addr.family = net_family::NET_IPv4;
        addr.ip.addr4 = addr4.s_addr;
    }
    else if (0 < inet_pton(AF_INET6, s.c_str(), &addr6))
    {
        addr.family = net_family::NET_IPv6;
        addr.ip.addr6[0] = addr6.s6_addr32[0];
        addr.ip.addr6[1] = addr6.s6_addr32[1];
        addr.ip.addr6[2] = addr6.s6_addr32[2];
        addr.ip.addr6[3] = addr6.s6_addr32[3];
    }
    return addr;
}
