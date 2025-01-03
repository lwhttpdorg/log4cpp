#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#endif

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include "log_net.h"
#include "log_pattern.h"
#include "tcp_output.h"

namespace log4cpp {
	std::atomic_bool tcp_output::running{true};

	net::socket_fd create_tcp_socket(const net::sock_addr &saddr) {
		net::socket_fd fd;
		if (saddr.addr.family == net::net_family::NET_IPv4) {
			fd = socket(AF_INET, SOCK_STREAM, 0);
		}
		else {
			fd = socket(AF_INET6, SOCK_STREAM, 0);
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

	void accept_worker(int listen_fd, std::unordered_set<net::socket_fd> &clients) {
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(listen_fd, &read_fds);
		timeval timeout{};
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		while (tcp_output::running) {
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
				net::socket_fd client_fd = accept(listen_fd, reinterpret_cast<struct sockaddr *>(&client_addr),
				                                  &client_addr_len);
				if (net::INVALID_FD != client_fd) {
					clients.insert(client_fd);
					FD_SET(client_fd, &read_fds);
				}
			}
			for (auto it = clients.begin(); it != clients.end();) {
				net::socket_fd fd = *it;
				if (FD_ISSET(fd, &tmp_fds)) {
					char buffer[LOG_LINE_MAX];
					buffer[0] = '\0';
#ifdef _WIN32
					const int len = recv(fd, buffer, sizeof(buffer), 0);
					if (len <= 0) {
#else
					const size_t len = recv(fd, buffer, sizeof(buffer), 0);
					if ( len == 0) {
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

	tcp_output::builder &tcp_output::builder::set_local_addr(const net::net_addr &addr) {
		if (this->instance == nullptr) {
			throw std::runtime_error("Call new_builder() first");
		}
		this->config.local_addr = addr;
		return *this;
	}

	tcp_output::builder &tcp_output::builder::set_port(unsigned short port) {
		if (this->instance == nullptr) {
			throw std::runtime_error("Call new_builder() first");
		}
		this->config.port = port;
		return *this;
	}

	std::shared_ptr<tcp_output> tcp_output::builder::build() {
		if (this->instance == nullptr) {
			throw std::runtime_error("Call new_builder() first");
		}
		net::sock_addr saddr;
		saddr.addr = this->config.local_addr;
		saddr.port = this->config.port;
		net::socket_fd server_fd = create_tcp_socket(saddr);
		if (net::INVALID_FD == server_fd) {
			throw std::runtime_error("Can not create tcp socket");
		}
		this->instance->fd = server_fd;
		this->instance->accept_thread = std::thread(accept_worker, server_fd, std::ref(this->instance->clients));
		return this->instance;
	}

	tcp_output::builder tcp_output::builder::new_builder() {
		builder builder = tcp_output::builder{};
		builder.instance = std::shared_ptr<tcp_output>(new tcp_output());
		return builder;
	}

	tcp_output::tcp_output() {
		this->fd = net::INVALID_FD;
	}

	tcp_output::~tcp_output() {
		running = false;
		this->accept_thread.join();
		if (net::INVALID_FD != this->fd) {
			net::close_socket(this->fd);
		}
		for (auto &client:this->clients) {
			net::close_socket(client);
		}
	}

	void tcp_output::log(log_level level, const char *fmt, va_list args) {
		char buffer[LOG_LINE_MAX];
		buffer[0] = '\0';
		const size_t used_len = log_pattern::format(buffer, sizeof(buffer), level, fmt, args);
		singleton_log_lock &lock = singleton_log_lock::get_instance();
		lock.lock();
		for (auto &client:this->clients) {
			(void)send(client, buffer, used_len, 0);
		}
		lock.unlock();
	}

	void tcp_output::log(log_level level, const char *fmt, ...) {
		char buffer[LOG_LINE_MAX];
		buffer[0] = '\0';
		const size_t used_len = log_pattern::format(buffer, sizeof(buffer), level, fmt);
		singleton_log_lock &lock = singleton_log_lock::get_instance();
		lock.lock();
		for (auto &client:this->clients) {
			(void)send(client, buffer, used_len, 0);
		}
		lock.unlock();
	}

	std::shared_ptr<tcp_output> tcp_output_config::get_instance(const tcp_output_config &config) {
		static std::shared_ptr<tcp_output> instance = nullptr;
		static log_lock instance_lock;
		if (instance == nullptr) {
			instance_lock.lock();
			if (instance == nullptr) {
				instance = tcp_output::builder::new_builder().set_local_addr(config.local_addr).set_port(
					config.port).build();
			}
			instance_lock.unlock();
		}
		return instance;
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, tcp_output_config const &obj) {
		json = boost::json::object{
			{"localAddr", net::to_string(obj.local_addr)},
			{"port", obj.port}
		};
	}

	tcp_output_config tag_invoke(boost::json::value_to_tag<tcp_output_config>, boost::json::value const &json) {
		tcp_output_config config;
		config.local_addr = net::net_addr(boost::json::value_to<std::string>(json.at("localAddr")));
		config.port = boost::json::value_to<unsigned short>(json.at("port"));
		return config;
	}
}
