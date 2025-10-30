#if defined(_WIN32)
// clang-format off
#include <winsock2.h>
#include <windows.h>
// clang-format on
#include <ws2tcpip.h>
#endif

#ifdef __linux__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#endif

#include "layout_pattern.h"
#include "log_net.h"
#include "tcp_appender.h"

namespace log4cpp {
	std::atomic_bool tcp_appender::running{true};

	net::socket_fd create_tcp_socket(const net::sock_addr &saddr) {
		net::socket_fd fd;
		if (saddr.addr.family == net::net_family::NET_IPv4) {
			fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		}
		else {
			fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
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
			if (bind(fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
				net::close_socket(fd);
				return net::INVALID_FD;
			}
		}
		else {
			sockaddr_in6 server_addr{};
			server_addr.sin6_family = AF_INET6;
			server_addr.sin6_port = htons(saddr.port);
			server_addr.sin6_addr = in6addr_any;
			if (bind(fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
				net::close_socket(fd);
				return net::INVALID_FD;
			}
		}
		if (listen(fd, 5) == -1) {
			net::close_socket(fd);
			return net::INVALID_FD;
		}
		return fd;
	}

	void accept_worker(net::socket_fd listen_fd, log_lock lock, std::unordered_set<net::socket_fd> &clients) {
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(listen_fd, &read_fds);
		timeval timeout{};
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		while (tcp_appender::running) {
			fd_set tmp_fds = read_fds;
			const int ret = select(listen_fd + 1, &tmp_fds, nullptr, nullptr, &timeout);
			if (ret == -1) {
				break;
			}
			if (ret == 0) {
				continue;
			}
			if (FD_ISSET(listen_fd, &tmp_fds)) {
				sockaddr_storage client_addr{};
				socklen_t client_addr_len = sizeof(client_addr);
				net::socket_fd client_fd =
					accept(listen_fd, reinterpret_cast<struct sockaddr *>(&client_addr), &client_addr_len);
				if (net::INVALID_FD != client_fd) {
#ifdef _DEBUG
					char client_ip[INET6_ADDRSTRLEN];
					unsigned short port;
					if (AF_INET == client_addr.ss_family) {
						const sockaddr_in *client_addr4 = reinterpret_cast<struct sockaddr_in *>(&client_addr);
						port = ntohs(client_addr4->sin_port);
						inet_ntop(client_addr4->sin_family, &client_addr4->sin_addr, client_ip, sizeof(client_ip));
					}
					else {
						const sockaddr_in6 *client_addr6 = reinterpret_cast<struct sockaddr_in6 *>(&client_addr);
						port = ntohs(client_addr6->sin6_port);
						inet_ntop(client_addr6->sin6_family, &client_addr6->sin6_addr, client_ip, sizeof(client_ip));
					}
					printf("New TCP client: %s@%hu\n", client_ip, port);
#endif
					std::lock_guard lock_guard(lock);
					clients.insert(client_fd);
					FD_SET(client_fd, &read_fds);
				}
			}
			std::lock_guard lock_guard(lock);
			for (auto it = clients.begin(); it != clients.end();) {
				net::socket_fd fd = *it;
				if (FD_ISSET(fd, &tmp_fds)) {
					char buffer[LOG_LINE_MAX];
					buffer[0] = '\0';
					ssize_t len = recv(fd, buffer, sizeof(buffer), 0);
					if (len < 0) {
#ifdef _WIN32
						shutdown(fd, SD_BOTH);
#else
						shutdown(fd, SHUT_RDWR);
#endif
						net::close_socket(fd);
						it = clients.erase(it);
						FD_CLR(fd, &read_fds);
					}
					else {
						++it;
					}
				}
				else {
					++it;
				}
			}
		}
	}

	tcp_appender::builder &tcp_appender::builder::set_local_addr(const net::net_addr &addr) {
		if (this->instance == nullptr) {
			throw std::runtime_error("Call tcp_appender::builder::new_builder() first");
		}
		this->config.set_local_addr(addr);
		return *this;
	}

	tcp_appender::builder &tcp_appender::builder::set_port(unsigned short port) {
		if (this->instance == nullptr) {
			throw std::runtime_error("Call tcp_appender::builder::new_builder() first");
		}
		this->config.set_port(port);
		return *this;
	}

	std::shared_ptr<tcp_appender> tcp_appender::builder::build() {
		if (this->instance == nullptr) {
			throw std::runtime_error("Call tcp_appender::builder::new_builder() first");
		}
		net::sock_addr saddr;
		saddr.addr = this->config.get_local_addr();
		saddr.port = this->config.get_port();
		net::socket_fd server_fd = create_tcp_socket(saddr);
		if (net::INVALID_FD == server_fd) {
			throw std::runtime_error("Can not create tcp socket");
		}
		this->instance->fd = server_fd;
		this->instance->accept_thread =
			std::thread(accept_worker, server_fd, this->instance->lock, std::ref(this->instance->clients));
		return this->instance;
	}

	tcp_appender::builder tcp_appender::builder::new_builder() {
		builder builder = tcp_appender::builder{};
		builder.instance = std::shared_ptr<tcp_appender>(new tcp_appender());
		return builder;
	}

	tcp_appender::tcp_appender() {
		this->fd = net::INVALID_FD;
	}

	tcp_appender::~tcp_appender() {
		running = false;
		this->accept_thread.join();
		if (net::INVALID_FD != this->fd) {
			net::close_socket(this->fd);
		}
		for (auto &client: this->clients) {
#ifdef _WIN32
			shutdown(client, SD_BOTH);
#else
			shutdown(client, SHUT_RDWR);
#endif
			net::close_socket(client);
		}
	}

	void tcp_appender::log(const char *msg, size_t msg_len) {
		std::lock_guard lock_guard(this->lock);
		for (auto &client: this->clients) {
			(void)send(client, msg, msg_len, 0);
		}
	}

	std::shared_ptr<tcp_appender> tcp_appender_config::instance = nullptr;
	log_lock tcp_appender_config::instance_lock;

	std::shared_ptr<tcp_appender> tcp_appender_config::get_instance(const tcp_appender_config &config) {
		if (instance == nullptr) {
			std::lock_guard lock(instance_lock);
			if (instance == nullptr) {
				instance = tcp_appender::builder::new_builder()
							   .set_local_addr(config.local_addr)
							   .set_port(config.port)
							   .build();
			}
		}
		return instance;
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, tcp_appender_config const &obj) {
		json = boost::json::object{{"local_addr", obj.local_addr.to_string()}, {"port", obj.port}};
	}

	tcp_appender_config tag_invoke(boost::json::value_to_tag<tcp_appender_config>, boost::json::value const &json) {
		tcp_appender_config config;
		config.local_addr = net::net_addr(boost::json::value_to<std::string>(json.at("local_addr")));
		config.port = boost::json::value_to<unsigned short>(json.at("port"));
		return config;
	}
}
