#include <sys/socket.h>
#include <netinet/in.h>
#include "tcp_output.h"


namespace log4cpp {

	std::atomic_bool tcp_output::running{true};

	int create_tcp_socket(const net::net_addr &addr, unsigned short port) {
		int fd;
		if (addr.family == net::net_family::NET_IPv4) {
			fd = socket(AF_INET, SOCK_STREAM, 0);
		}
		else {
			fd = socket(AF_INET6, SOCK_STREAM, 0);
		}
		if (fd == -1) {
			return -1;
		}
		int opt = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
		if (addr.family == net::net_family::NET_IPv4) {
			struct sockaddr_in server_addr{};
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(port);
			server_addr.sin_addr.s_addr = htonl(addr.ip.addr4);
			if (bind(fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
				close(fd);
				return -1;
			}
		}
		else {
			sockaddr_in6 server_addr{};
			server_addr.sin6_family = AF_INET6;
			server_addr.sin6_port = htons(port);
			server_addr.sin6_addr = in6addr_any;
			if (bind(fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
				close(fd);
				return -1;
			}
		}
		if (listen(fd, 5) == -1) {
			close(fd);
			return -1;
		}
		return fd;
	}

	void accept_worker(int listen_fd, std::unordered_set<int> &clients) {
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(listen_fd, &read_fds);
		struct timeval timeout{};
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		while (tcp_output::running) {
			fd_set tmp_fds = read_fds;
			int ret = select(listen_fd + 1, &tmp_fds, nullptr, nullptr, &timeout);
			if (ret == -1) {
				break;
			}
			if (ret == 0) {
				continue;
			}
			if (FD_ISSET(listen_fd, &tmp_fds)) {
				struct sockaddr_storage client_addr{};
				socklen_t client_addr_len = sizeof(client_addr);
				int client_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_addr_len);
				if (-1 != client_fd) {
					clients.insert(client_fd);
					FD_SET(client_fd, &read_fds);
				}
			}
			for (auto client: clients) {
				if (FD_ISSET(client, &tmp_fds)) {
					char buffer[LOG_LINE_MAX];
					buffer[0] = '\0';
					size_t len = recv(client, buffer, sizeof(buffer), 0);
					if (len == 0) {
						close(client);
						clients.erase(client);
						FD_CLR(client, &read_fds);
					}
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
		int server_fd = create_tcp_socket(this->config.local_addr, this->config.port);
		if (server_fd == -1) {
			throw std::runtime_error("Can not create tcp socket");
		}
		this->instance->fd = server_fd;
		this->instance->accept_thread = std::thread(accept_worker, server_fd, std::ref(this->instance->clients));
		return this->instance;
	}

	tcp_output::builder tcp_output::builder::new_builder() {
		tcp_output::builder builder = tcp_output::builder{};
		builder.instance = std::shared_ptr<tcp_output>(new tcp_output());
		return builder;
	}

	tcp_output::tcp_output() {
		this->fd = -1;
	}

	tcp_output::~tcp_output() {
		log4cpp::tcp_output::running = false;
		this->accept_thread.join();
		if (this->fd != -1) {
			close(this->fd);
		}
		for (auto &client: this->clients) {
			close(client);
		}
	}

	void tcp_output::log(log_level level, const char *fmt, va_list args) {
		char buffer[LOG_LINE_MAX];
		buffer[0] = '\0';
		size_t used_len = log_pattern::format(buffer, sizeof(buffer), level, fmt, args);
		singleton_log_lock &lock = singleton_log_lock::get_instance();
		lock.lock();
		for (auto &client: this->clients) {
			send(client, buffer, used_len, 0);
		}
		lock.unlock();
	}

	void tcp_output::log(log_level level, const char *fmt, ...) {
		char buffer[LOG_LINE_MAX];
		buffer[0] = '\0';
		size_t used_len = log_pattern::format(buffer, sizeof(buffer), level, fmt);
		singleton_log_lock &lock = singleton_log_lock::get_instance();
		lock.lock();
		for (auto &client: this->clients) {
			(void) send(client, buffer, used_len, 0);
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
		json = boost::json::object{{"localAddr", log4cpp::net::to_string(obj.local_addr)},
		                           {"port",      obj.port}};
	}

	tcp_output_config tag_invoke(boost::json::value_to_tag<tcp_output_config>, boost::json::value const &json) {
		tcp_output_config config;
		config.local_addr = log4cpp::net::net_addr(boost::json::value_to<std::string>(json.at("localAddr")));
		config.port = boost::json::value_to<unsigned short>(json.at("port"));
		return config;
	}
}
