#pragma once

#include <ostream>
#include "../include/log4cpp.hpp"
#include "console_output.h"
#include "file_output.h"
#include "tcp_output.h"
#include "udp_output.h"

namespace log4cpp {

	constexpr unsigned char CONSOLE_OUT_CFG = 0x01;
	constexpr unsigned char FILE_OUT_CFG = 0x02;
	constexpr unsigned char TCP_OUT_CFG = 0x04;
	constexpr unsigned char UDP_OUT_CFG = 0x08;

	class output_config {
		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, output_config const &obj);

		friend output_config tag_invoke(boost::json::value_to_tag<output_config>, boost::json::value const &json);

	public:
		unsigned char OUT_FLAGS{};
		console_output_config console_cfg;
		file_output_config file_cfg;
		//tcp_output_config tcp_cfg;
		//udp_output_config udp_cfg;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, output_config const &obj);

	output_config tag_invoke(boost::json::value_to_tag<output_config>, boost::json::value const &json);

	class logger_config {
	public:
		logger_config();

		logger_config(const logger_config &other);

		logger_config(logger_config &&other) noexcept;

		logger_config &operator=(const logger_config &other);

		logger_config &operator=(logger_config &&other) noexcept;

		[[nodiscard]] std::string get_logger_name() const;

		[[nodiscard]] log_level get_logger_level() const;

		[[nodiscard]] unsigned char get_outputs() const;

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, logger_config const &obj);

		friend logger_config tag_invoke(boost::json::value_to_tag<logger_config>, boost::json::value const &json);

	private:
		std::string name;
		log_level level;
		unsigned char _outputs{};
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, logger_config const &obj);

	logger_config tag_invoke(boost::json::value_to_tag<logger_config>, boost::json::value const &json);

	class log4cpp_config {
	public:
		static log4cpp_config load_config(const std::string &json_file);

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, log4cpp_config const &obj);

		friend log4cpp_config tag_invoke(boost::json::value_to_tag<log4cpp_config>, boost::json::value const &json);

		friend std::string serialize(log4cpp_config const &obj);

		log4cpp_config() = default;

		log4cpp_config(std::string _pattern, output_config &o, const std::vector<logger_config> &l, logger_config root);

		log4cpp_config(const log4cpp_config &other);

		log4cpp_config(log4cpp_config &&other) noexcept;

		log4cpp_config &operator=(const log4cpp_config &other);

		log4cpp_config &operator=(log4cpp_config &&other) noexcept;

		friend class logger_manager;

	private:
		std::string pattern;   // pattern
		output_config output{};  // logOutPut
		std::vector<logger_config> loggers; // loggers
		logger_config root_logger;          // rootLogger
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, log4cpp_config const &obj);

	log4cpp_config tag_invoke(boost::json::value_to_tag<log4cpp_config>, boost::json::value const &json);

	std::string serialize(log4cpp_config const &obj);
}
