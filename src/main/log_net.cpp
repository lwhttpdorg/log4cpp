#include <stdexcept>

#if defined(__linux__)

#include <netinet/in.h>
#include <arpa/inet.h>

#endif

#if defined(_WIN32)

#include <WS2tcpip.h>
#include <ws2ipdef.h>

#endif

#include "log_net.h"

using namespace log4cpp::net;

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
		this->addr.addr6[0] = addr6.u.Word[0];
		this->addr.addr6[1] = addr6.u.Word[1];
		this->addr.addr6[2] = addr6.u.Word[2];
		this->addr.addr6[3] = addr6.u.Word[3];
#endif
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
	else {
		return ip.addr6[0] == rhs.ip.addr6[0] && ip.addr6[1] == rhs.ip.addr6[1] && ip.addr6[2] == rhs.ip.addr6[2] &&
		       ip.addr6[3] == rhs.ip.addr6[3];
	}
}

bool net_addr::operator!=(const net_addr &rhs) const {
	return !(rhs == *this);
}

std::string log4cpp::net::to_string(const net_addr &addr) {
	std::string s;
	if (addr.family == net_family::NET_IPv4) {
		char buf[INET_ADDRSTRLEN];
		unsigned char a, b, c, d;
		a = (addr.ip.addr4 >> 24) & 0xff;
		b = (addr.ip.addr4 >> 16) & 0xff;
		c = (addr.ip.addr4 >> 8) & 0xff;
		d = addr.ip.addr4 & 0xff;
		snprintf(buf, sizeof(buf), "%u.%u.%u.%u", a, b, c, d);
		s = std::string{buf};
	}
	else if (addr.family == net_family::NET_IPv6) {
		char buf[INET6_ADDRSTRLEN];
		int len = 0;
		for (auto x: addr.ip.addr6) {
			unsigned short a = (x >> 16), b = x & 0xffff;
			int l = snprintf(buf + len, sizeof(buf) - len, "%x%x:", a, b);
			if (l > 0) {
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
		throw std::invalid_argument(
				"Invalid addr family \"" + std::to_string(static_cast<int> (addr.family)) + "\"");
	}
	return s;
}

sock_addr::sock_addr() {
	addr = net_addr{};
	port = 0;
}

sock_addr::sock_addr(const char *ip, unsigned short port) {
	this->addr = net_addr(ip);
	this->port = port;
}

sock_addr::sock_addr(const std::string &ip, unsigned short port) : sock_addr(ip.c_str(), port) {
}

bool sock_addr::operator==(const sock_addr &rhs) const {
	return addr == rhs.addr &&
	       port == rhs.port;
}

bool sock_addr::operator!=(const sock_addr &rhs) const {
	return !(rhs == *this);
}
