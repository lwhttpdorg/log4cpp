#pragma once

#include <boost/json.hpp>
#include <memory>
#include <thread>
#include <unordered_set>

#include "log_appender.h"
#include "log_lock.h"
#include "log_net.h"

namespace log4cpp {
	class udp_appender;

	class udp_appender_config {
	public:
		/**
		 * @brief Get an instance of udp_appender_instance with the given configuration
		 * @param config: UDP appender configuration
		 * @return UDP appender instance
		 */
		static std::shared_ptr<log_appender> build_instance(const udp_appender_config &config);

		[[nodiscard]] net::net_addr get_local_addr() const {
			return local_addr;
		}

		void set_local_addr(const net::net_addr &addr) {
			this->local_addr = addr;
		}

		[[nodiscard]] unsigned short get_port() const {
			return port;
		}

		void set_port(unsigned short p) {
			this->port = p;
		}

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, udp_appender_config const &obj);

		friend udp_appender_config tag_invoke(boost::json::value_to_tag<udp_appender_config>,
											  boost::json::value const &json);

	private:
		net::net_addr local_addr{};
		unsigned short port{0};
		static std::shared_ptr<log_appender> instance;
		static log_lock instance_lock;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, udp_appender_config const &obj);

	udp_appender_config tag_invoke(boost::json::value_to_tag<udp_appender_config>, boost::json::value const &json);

	class udp_appender final: public log_appender {
	public:
		class builder {
		public:
			/**
			 * @brief Set the local address for the UDP appender
			 * @param addr: Local address
			 * @return Reference to the builder
			 */
			builder &set_local_addr(const net::net_addr &addr);

			/**
			 * @brief Set the port for the UDP appender
			 * @param port: Port number
			 * @return Reference to the builder
			 */
			builder &set_port(unsigned short port);

			/**
			 * @brief Build the UDP appender instance
			 * @return UDP appender instance
			 */
			std::shared_ptr<udp_appender> build();

			/**
			 * @brief Create an instance of the UDP appender builder
			 * @return UDP appender builder
			 */
			static builder new_builder();

		private:
			builder() = default;

		private:
			udp_appender_config config;
			std::shared_ptr<udp_appender> instance{nullptr};
		};

		udp_appender(const udp_appender &other) = delete;

		udp_appender(udp_appender &&other) = delete;

		udp_appender &operator=(const udp_appender &other) = delete;

		udp_appender &operator=(udp_appender &&other) = delete;

		void log(const char *msg, size_t msg_len) override;

		~udp_appender() override;

	private:
		udp_appender();

	public:
		/* UDP server accept thread running flag */
		static std::atomic_bool running;

	private:
		/* UDP server socket fd */
		net::socket_fd fd;
		/* Address of clients */
		std::unordered_set<net::sock_addr> clients;
		/* UDP server accept thread */
		std::thread accept_thread;
		log_lock lock;
	};
}
