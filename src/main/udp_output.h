#pragma once

#include <memory>
#include <thread>
#include <unordered_set>
#include <boost/json.hpp>

#include "log_net.h"
#include "log_output.h"

namespace log4cpp {
	class udp_output;

	class udp_output_config {
	public:
		net::net_addr local_addr{};
		unsigned short port{0};

	public:
		/**
		 * @brief Get an instance of udp_output with the given configuration
		 * @param config UDP output configuration
		 * @return UDP output instance
		 */
		static std::shared_ptr<udp_output> get_instance(const udp_output_config &config);

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, udp_output_config const &obj);

		friend udp_output_config
		tag_invoke(boost::json::value_to_tag<udp_output_config>, boost::json::value const &json);
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, udp_output_config const &obj);

	udp_output_config tag_invoke(boost::json::value_to_tag<udp_output_config>, boost::json::value const &json);

	class udp_output final : public log_output {
	public:
		class builder {
		public:
			/**
			 * @brief Set the local address for the UDP output
			 * @param addr Local address
			 * @return Reference to the builder
			 */
			builder &set_local_addr(const net::net_addr &addr);

			/**
			 * @brief Set the port for the UDP output
			 * @param port Port number
			 * @return Reference to the builder
			 */
			builder &set_port(unsigned short port);

			/**
			 * @brief Build the UDP output instance
			 * @return UDP output instance
			 */
			std::shared_ptr<udp_output> build();

			/**
			 * @brief Create an instance of the UDP output builder
			 * @return UDP output builder
			 */
			static builder new_builder();

		private:
			builder() = default;

		private:
			udp_output_config config;
			std::shared_ptr<udp_output> instance{nullptr};
		};

		udp_output(const udp_output &other) = delete;

		udp_output(udp_output &&other) = delete;

		udp_output &operator=(const udp_output &other) = delete;

		udp_output &operator=(udp_output &&other) = delete;

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

		~udp_output() override;

	private:
		udp_output();

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
	};
}
