#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#endif

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include "layout_pattern.h"
#include "udp_appender.h"

namespace log4cpp {
	const char *UDP_OUTPUT_HELLO = "hello";
	const char *UDP_OUTPUT_GOODBYE = "bye";

	std::atomic_bool udp_appender::running{true};

	net::socket_fd create_udp_socket(const net::sock_addr &saddr) {
		net::socket_fd fd;
		if (saddr.addr.family == net::net_family::NET_IPv4) {
			fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		}
		else {
			fd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
		}
		if (fd == net::INVALID_FD) {
			return net::INVALID_FD;
		}
		int opt = 1;
#ifdef _WIN32
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&opt), sizeof(opt));
#endif
#ifdef __linux__
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif
		if (saddr.addr.family == net::net_family::NET_IPv4) {
			sockaddr_in server_addr{};
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(saddr.port);
			server_addr.sin_addr.s_addr = htonl(saddr.addr.ip.addr4);
			if (bind(fd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
				net::close_socket(fd);
				return net::INVALID_FD;
			}
		}
		else {
			sockaddr_in6 server_addr{};
			server_addr.sin6_family = AF_INET6;
			server_addr.sin6_port = htons(saddr.port);
			server_addr.sin6_addr = in6addr_any;
			if (bind(fd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
				net::close_socket(fd);
				return net::INVALID_FD;
			}
		}
		return fd;
	}

	void accept_worker(net::socket_fd listen_fd, log_lock lock, std::unordered_set<net::sock_addr> &clients) {
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(listen_fd, &read_fds);
		timeval timeout{};
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		while (udp_appender::running) {
			fd_set tmp_fds = read_fds;
			const int ret = select(listen_fd + 1, &tmp_fds, nullptr, nullptr, &timeout);
			if (ret == -1) {
				break;
			}
			if (ret == 0) {
				continue;
			}
			if (FD_ISSET(listen_fd, &tmp_fds)) {
				char buffer[LOG_LINE_MAX];
				buffer[0] = '\0';
				sockaddr_storage client_addr{};
				socklen_t client_addr_len = sizeof(client_addr);
				ssize_t len = recvfrom(listen_fd, buffer, sizeof(buffer) - 1, 0,
										reinterpret_cast<struct sockaddr *>(&client_addr),
										&client_addr_len);
				if (len < 0) {
					continue;
				}

				net::sock_addr saddr{};
				if (client_addr.ss_family == AF_INET) {
					const sockaddr_in *client_addr_in = reinterpret_cast<struct sockaddr_in *>(&client_addr);
					saddr.addr.family = net::net_family::NET_IPv4;
					saddr.addr.ip.addr4 = ntohl(client_addr_in->sin_addr.s_addr);
					saddr.port = ntohs(client_addr_in->sin_port);
				}
				else {
					const sockaddr_in6 *client_addr_in6 = reinterpret_cast<struct sockaddr_in6 *>(&client_addr);
					saddr.addr.family = net::net_family::NET_IPv6;
					memcpy(saddr.addr.ip.addr6, &client_addr_in6->sin6_addr, sizeof(saddr.addr.ip.addr6));
					saddr.port = ntohs(client_addr_in6->sin6_port);
				}
#ifdef _DEBUG
				printf("New UDP client: %hs\n", saddr.to_string().c_str());
#endif
				buffer[len] = '\0';
				if (strcmp(buffer, UDP_OUTPUT_HELLO) == 0) {
					std::lock_guard lock_guard(lock);
					clients.insert(saddr);
				}
				else if (strcmp(buffer, UDP_OUTPUT_GOODBYE) == 0) {
					std::lock_guard lock_guard(lock);
					clients.erase(saddr);
				}
			}
		}
	}

	udp_appender::builder &udp_appender::builder::set_local_addr(const net::net_addr &addr) {
		if (this->instance == nullptr) {
			throw std::runtime_error("Call udp_appender::builder::new_builder() first");
		}
		this->config.set_local_addr(addr);
		return *this;
	}

	udp_appender::builder &udp_appender::builder::set_port(unsigned short port) {
		if (this->instance == nullptr) {
			throw std::runtime_error("Call udp_appender::builder::new_builder() first");
		}
		this->config.set_port(port);
		return *this;
	}

	std::shared_ptr<udp_appender> udp_appender::builder::build() {
		if (this->instance == nullptr) {
			throw std::runtime_error("Call udp_appender::builder::new_builder() first");
		}
		net::sock_addr saddr;
		saddr.addr = this->config.get_local_addr();
		saddr.port = this->config.get_port();
		net::socket_fd server_fd = create_udp_socket(saddr);
		if (server_fd == net::INVALID_FD) {
			throw std::runtime_error("Can not create tcp socket");
		}
		this->instance->fd = server_fd;
		this->instance->accept_thread = std::thread(accept_worker, server_fd, this->instance->lock,
													std::ref(this->instance->clients));
		return this->instance;
	}

	udp_appender::builder udp_appender::builder::new_builder() {
		builder builder = udp_appender::builder{};
		builder.instance = std::shared_ptr<udp_appender>(new udp_appender());
		return builder;
	}

	udp_appender::udp_appender() {
		this->fd = net::INVALID_FD;
	}

	udp_appender::~udp_appender() {
		running = false;
		this->accept_thread.join();
		if (this->fd != net::INVALID_FD) {
			net::close_socket(this->fd);
		}
	}

	void udp_appender::log(const char *msg, size_t msg_len) {
		std::lock_guard lock_guard(this->lock);
		for (auto &client:this->clients) {
			if (client.addr.family == net::net_family::NET_IPv4) {
				sockaddr_in client_addr{};
				client_addr.sin_family = AF_INET;
				client_addr.sin_port = htons(client.port);
				client_addr.sin_addr.s_addr = htonl(client.addr.ip.addr4);
				(void)sendto(this->fd, msg, msg_len, 0, reinterpret_cast<sockaddr *>(&client_addr),
							sizeof(client_addr));
			}
			else {
				sockaddr_in6 client_addr{};
				client_addr.sin6_family = AF_INET6;
				client_addr.sin6_port = htons(client.port);
				client_addr.sin6_addr = in6addr_any;
				(void)sendto(this->fd, msg, msg_len, 0, reinterpret_cast<sockaddr *>(&client_addr),
							sizeof(client_addr));
			}
		}
	}

	std::shared_ptr<udp_appender> udp_appender_config::instance = nullptr;
	log_lock udp_appender_config::instance_lock;

	std::shared_ptr<udp_appender> udp_appender_config::get_instance(const udp_appender_config &config) {
		if (instance == nullptr) {
			std::lock_guard lock(udp_appender_config::instance_lock);
			if (instance == nullptr) {
				instance = udp_appender::builder::new_builder().set_local_addr(config.local_addr).set_port(
					config.port).build();
			}
		}
		return instance;
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const udp_appender_config &obj) {
		json = boost::json::object{
			{"local_addr", obj.local_addr.to_string()},
			{"port", obj.port}
		};
	}

	udp_appender_config tag_invoke(boost::json::value_to_tag<udp_appender_config>, boost::json::value const &json) {
		udp_appender_config config;
		config.local_addr = net::net_addr(boost::json::value_to<std::string>(json.at("local_addr")));
		config.port = boost::json::value_to<unsigned short>(json.at("port"));
		return config;
	}
}
