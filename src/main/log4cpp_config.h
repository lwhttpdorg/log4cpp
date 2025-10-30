#pragma once

#include "../include/log4cpp.hpp"
#include "console_appender.h"
#include "file_appender.h"
#include "tcp_appender.h"
#include "udp_appender.h"

namespace log4cpp {
	class appender_config {
		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, appender_config const &obj);

		friend appender_config tag_invoke(boost::json::value_to_tag<appender_config>, boost::json::value const &json);

	public:
		[[nodiscard]] const console_appender_config *get_console_cfg() const {
			if (APPENDER_FLAGS & CONSOLE_APPENDER_FLAG) {
				return &console_cfg;
			}
			return nullptr;
		}

		[[nodiscard]] const file_appender_config *get_file_cfg() const {
			if (APPENDER_FLAGS & FILE_APPENDER_FLAG) {
				return &file_cfg;
			}
			return nullptr;
		}

		[[nodiscard]] const tcp_appender_config *get_tcp_cfg() const {
			if (APPENDER_FLAGS & TCP_APPENDER_FLAG) {
				return &tcp_cfg;
			}
			return nullptr;
		}

		[[nodiscard]] const udp_appender_config *get_udp_cfg() const {
			if (APPENDER_FLAGS & UDP_APPENDER_FLAG) {
				return &udp_cfg;
			}
			return nullptr;
		}

		unsigned char APPENDER_FLAGS{};
		console_appender_config console_cfg;
		file_appender_config file_cfg;
		tcp_appender_config tcp_cfg;
		udp_appender_config udp_cfg;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, appender_config const &obj);

	appender_config tag_invoke(boost::json::value_to_tag<appender_config>, boost::json::value const &json);

	class logger_config {
	public:
		logger_config();

		logger_config(const logger_config &other);

		logger_config(logger_config &&other) noexcept;

		logger_config &operator=(const logger_config &other);

		logger_config &operator=(logger_config &&other) noexcept;

		friend bool operator<(const logger_config &lhs, const logger_config &rhs) {
			if (lhs.name < rhs.name) return true;
			if (rhs.name < lhs.name) return false;
			if (lhs.level < rhs.level) return true;
			if (rhs.level < lhs.level) return false;
			return lhs.logger_flag < rhs.logger_flag;
		}

		friend bool operator<=(const logger_config &lhs, const logger_config &rhs) {
			return rhs >= lhs;
		}

		friend bool operator>(const logger_config &lhs, const logger_config &rhs) {
			return rhs < lhs;
		}

		friend bool operator>=(const logger_config &lhs, const logger_config &rhs) {
			return !(lhs < rhs);
		}

		friend bool operator==(const logger_config &lhs, const logger_config &rhs) {
			return lhs.name == rhs.name && lhs.level == rhs.level && lhs.logger_flag == rhs.logger_flag;
		}

		friend bool operator!=(const logger_config &lhs, const logger_config &rhs) {
			return !(lhs == rhs);
		}

		/**
		 * @brief Get the logger name
		 * @return The logger name
		 */
		[[nodiscard]] std::string get_logger_name() const;

		void set_name(const std::string &_name) {
			this->name = _name;
		}

		/* Get the logger level */
		[[nodiscard]] log_level get_logger_level() const;

		void set_level(log_level _level) {
			this->level = _level;
		}

		/* Get the logger flags */
		[[nodiscard]] unsigned char get_logger_flag() const;

		void set_logger_flag(unsigned char flags) {
			this->logger_flag = flags;
		}

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, logger_config const &obj);

		friend logger_config tag_invoke(boost::json::value_to_tag<logger_config>, boost::json::value const &json);

	private:
		/* Logger name */
		std::string name;
		/* Logger level */
		log_level level;
		/* Layout flag */
		unsigned char logger_flag{};
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, logger_config const &obj);

	logger_config tag_invoke(boost::json::value_to_tag<logger_config>, boost::json::value const &json);

	class log4cpp_config {
	public:
		/**
		 * @brief Load the configuration from a JSON file
		 * @param json_file: The JSON file
		 * @return The configuration
		 */
		[[nodiscard]]
		static log4cpp_config load_config(const std::string &json_file);

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, log4cpp_config const &obj);

		friend log4cpp_config tag_invoke(boost::json::value_to_tag<log4cpp_config>, boost::json::value const &json);

		/**
		 * @brief Serialize the configuration to a JSON string
		 * @param obj: The configuration
		 * @return The JSON string
		 */
		static std::string serialize(const log4cpp_config &obj);

		log4cpp_config() = default;

		log4cpp_config(std::string _pattern, const appender_config &o, const std::vector<logger_config> &l,
					   logger_config root);

		log4cpp_config(const log4cpp_config &other);

		log4cpp_config(log4cpp_config &&other) noexcept;

		log4cpp_config &operator=(const log4cpp_config &other);

		log4cpp_config &operator=(log4cpp_config &&other) noexcept;

		[[nodiscard]] const std::string &get_logger_pattern() const;

		[[nodiscard]] const appender_config &get_appender() const;

		[[nodiscard]] const std::vector<logger_config> &get_loggers() const;

		[[nodiscard]] const logger_config &get_root_logger() const;

		friend class logger_manager;

	private:
		std::string logger_pattern; // logger_pattern
		appender_config appender{}; // appenders
		std::vector<logger_config> loggers; // loggers
		logger_config root_logger; // root_logger
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, log4cpp_config const &obj);

	log4cpp_config tag_invoke(boost::json::value_to_tag<log4cpp_config>, boost::json::value const &json);
}
