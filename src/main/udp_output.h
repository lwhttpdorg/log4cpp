#pragma once

#include <memory>
#include <thread>
#include <unordered_set>
#include "log_net.h"
#include "log_output.h"

namespace log4cpp {
	class udp_output;

	class udp_output_config {
	public:
		net::net_addr local_addr{};
		unsigned short port{0};
	public:
		static std::shared_ptr<udp_output> get_instance(const udp_output_config &config);

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, udp_output_config const &obj);

		friend udp_output_config
		tag_invoke(boost::json::value_to_tag<udp_output_config>, boost::json::value const &json);
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, udp_output_config const &obj);

	udp_output_config tag_invoke(boost::json::value_to_tag<udp_output_config>, boost::json::value const &json);

	class udp_output : public log_output {
	public:
		class builder {
		public:
			builder &set_local_addr(const net::net_addr &addr);

			builder &set_port(unsigned short port);

			std::shared_ptr<udp_output> build();

			static builder new_builder();

		private:
			udp_output_config config;
			std::shared_ptr<udp_output> instance{nullptr};
		};

		udp_output(const udp_output &other) = delete;

		udp_output(udp_output &&other) = delete;

		udp_output &operator=(const udp_output &other) = delete;

		udp_output &operator=(udp_output &&other) = delete;

		void log(log_level level, const char *__restrict fmt, va_list args) override;

		void log(log_level level, const char *__restrict fmt, ...) override;

		~udp_output() override;

	private:
		udp_output();

	public:
		static std::atomic_bool running;
	private:
		int fd;
		std::unordered_set<net::sock_addr> clients;
		std::thread accept_thread;
	};
}
