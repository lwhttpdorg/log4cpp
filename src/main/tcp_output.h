#pragma once

#include <unordered_set>
#include <memory>
#include <thread>
#include "log_net.h"
#include "log_output.h"

namespace log4cpp {
	class tcp_output;

	class tcp_output_config {
	public:
		net::net_addr local_addr{};
		unsigned short port{0};

	public:
		/**
		 * @brief Get an instance of tcp_output with the given configuration
		 * @param config TCP output configuration
		 * @return TCP output instance
		 */
		static std::shared_ptr<tcp_output> get_instance(const tcp_output_config &config);

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, tcp_output_config const &obj);

		friend tcp_output_config
		tag_invoke(boost::json::value_to_tag<tcp_output_config>, boost::json::value const &json);
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, tcp_output_config const &obj);

	tcp_output_config tag_invoke(boost::json::value_to_tag<tcp_output_config>, boost::json::value const &json);


	class tcp_output final : public log_output {
	public:
		class builder {
		public:
			/**
			 * @brief Set the local address for the TCP output
			 * @param addr Local address
			 * @return Reference to the builder
			 */
			builder &set_local_addr(const net::net_addr &addr);

			/**
			 * @brief Set the port for the TCP output
			 * @param port Port number
			 * @return Reference to the builder
			 */
			builder &set_port(unsigned short port);

			/**
			 * @brief Build the TCP output instance
			 * @return TCP output instance
			 */
			std::shared_ptr<tcp_output> build();

			/**
			 * @brief Create a new TCP output builder
			 * @return TCP output builder instance
			 */
			static builder new_builder();

		private:
			builder() = default;

		private:
			tcp_output_config config;
			std::shared_ptr<tcp_output> instance{nullptr};
		};

		tcp_output(const tcp_output &other) = delete;

		tcp_output(tcp_output &&other) = delete;

		tcp_output &operator=(const tcp_output &other) = delete;

		tcp_output &operator=(tcp_output &&other) = delete;

		/**
		 * @brief Write log message with the given log level
		 * @param level Log level
		 * @param fmt Format string
		 * @param args Arguments
		 */
		void log(log_level level, const char *__restrict fmt, va_list args) override;

		/**
		 * @brief Write log message with the given log level
		 * @param level Log level
		 * @param fmt Format string
		 * @param ... Arguments
		 */
		void log(log_level level, const char *__restrict fmt, ...) override;

		~tcp_output() override;

	private:
		tcp_output();

	public:
		/* TCP server accept thread running flag */
		static std::atomic_bool running;

	private:
		/* TCP server listen socket */
		net::socket_fd fd;
		/* Socket fds of connected clients */
		std::unordered_set<net::socket_fd> clients;
		/* Thread for accepting client connections */
		std::thread accept_thread;
	};
}
