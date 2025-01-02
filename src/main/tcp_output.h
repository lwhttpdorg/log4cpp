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
		static std::shared_ptr<tcp_output> get_instance(const tcp_output_config &config);

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, tcp_output_config const &obj);

		friend tcp_output_config
		tag_invoke(boost::json::value_to_tag<tcp_output_config>, boost::json::value const &json);
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, tcp_output_config const &obj);

	tcp_output_config tag_invoke(boost::json::value_to_tag<tcp_output_config>, boost::json::value const &json);


	class tcp_output : public log_output {
	public:
		class builder {
		public:
			builder &set_local_addr(const net::net_addr &addr);

			builder &set_port(unsigned short port);

			std::shared_ptr<tcp_output> build();

			static builder new_builder();

		private:
			tcp_output_config config;
			std::shared_ptr<tcp_output> instance{nullptr};
		};

		tcp_output(const tcp_output &other) = delete;

		tcp_output(tcp_output &&other) = delete;

		tcp_output &operator=(const tcp_output &other) = delete;

		tcp_output &operator=(tcp_output &&other) = delete;

		void log(log_level level, const char *__restrict fmt, va_list args) override;

		void log(log_level level, const char *__restrict fmt, ...) override;

		~tcp_output() override;

	private:
		tcp_output();

	public:
		static std::atomic_bool running;
	private:
		int fd;
		std::unordered_set<int> clients;
		std::thread accept_thread;
	};
}
