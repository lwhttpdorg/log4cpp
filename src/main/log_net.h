#pragma once

#include <string>

#if defined(_WIN32)

#include <winsock.h>

#endif

namespace log4cpp::net {
#if defined(_MSC_VER) || defined(_WIN32)
	constexpr SOCKET INVALID_FD = INVALID_SOCKET;
#endif
#if defined(__linux__)
	constexpr int INVALID_FD = -1;
#endif

	enum class net_family {
		NET_IPv4, NET_IPv6
	};

	class net_addr {
	public:
		union addr_u {
			unsigned int addr4;
			unsigned int addr6[4];
		};

		net_family family{net_family::NET_IPv4};
		addr_u ip{};
	public:
		net_addr();

		explicit net_addr(const char *addr);

		explicit net_addr(const std::string &addr);

		bool operator==(const net_addr &rhs) const;

		bool operator!=(const net_addr &rhs) const;

		friend std::string to_string(const net_addr &addr);
	};

	std::string to_string(const net_addr &addr);

	class sock_addr {
	public:
		net_addr addr{};
		unsigned short port{0};
	public:
		sock_addr();

		explicit sock_addr(const char *ip, unsigned short port);

		explicit sock_addr(const std::string &ip, unsigned short port);

		bool operator==(const sock_addr &rhs) const;

		bool operator!=(const sock_addr &rhs) const;
	};
}

namespace std {
	template<>
	struct hash<log4cpp::net::net_addr> {
		std::size_t operator()(const log4cpp::net::net_addr &addr) const noexcept {
			std::size_t h = 0;
			if (addr.family == log4cpp::net::net_family::NET_IPv4) {
				h = std::hash<unsigned int>{}(addr.ip.addr4);
			}
			else {
				for (auto x: addr.ip.addr6) {
					h ^= std::hash<unsigned int>{}(x);
				}
			}
			return h;
		}
	};

	template<>
	struct hash<log4cpp::net::sock_addr> {
		std::size_t operator()(const log4cpp::net::sock_addr &saddr) const noexcept {
			std::size_t h = std::hash<log4cpp::net::net_addr>{}(saddr.addr);
			h ^= std::hash<unsigned short>{}(saddr.port);
			return h;
		}
	};
}
