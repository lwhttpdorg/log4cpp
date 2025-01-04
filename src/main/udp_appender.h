#pragma once

#include <memory>
#include <thread>
#include <unordered_set>
#include <boost/json.hpp>

#include "log_net.h"
#include "log_appender.h"

namespace log4cpp {
	class udp_appender;

	class udp_appender_config {
	public:
		net::net_addr local_addr{};
		unsigned short port{0};
		static std::shared_ptr<udp_appender> instance;
		static log_lock instance_lock;
	public:
		/**
		 * @brief Get an instance of udp_appender with the given configuration
		 * @param config UDP appender configuration
		 * @return UDP appender instance
		 */
		static std::shared_ptr<udp_appender> get_instance(const udp_appender_config &config);

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, udp_appender_config const &obj);

		friend udp_appender_config
		tag_invoke(boost::json::value_to_tag<udp_appender_config>, boost::json::value const &json);
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, udp_appender_config const &obj);

	udp_appender_config tag_invoke(boost::json::value_to_tag<udp_appender_config>, boost::json::value const &json);

	class udp_appender final : public log_appender {
	public:
		class builder {
		public:
			/**
			 * @brief Set the local address for the UDP appender
			 * @param addr Local address
			 * @return Reference to the builder
			 */
			builder &set_local_addr(const net::net_addr &addr);

			/**
			 * @brief Set the port for the UDP appender
			 * @param port Port number
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

		/**
		 * @brief Write a log message with the given log level
		 * @param level Log level
		 * @param fmt Format string
		 * @param args Arguments
		 */
		void log(log_level level, const char *__restrict fmt, va_list args) override;

		/**
		 * @brief Write a log message with the given log level
		 * @param level Log level
		 * @param fmt Format string
		 * @param ... Arguments
		 */
		void log(log_level level, const char *__restrict fmt, ...) override;

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
