#pragma once

#include "../include/log4cpp.hpp"
#include "console_appender.h"
#include "file_appender.h"
#include "tcp_appender.h"
#include "udp_appender.h"

namespace log4cpp {
	constexpr unsigned char CONSOLE_APPENDER_CFG = 0x01;
	constexpr unsigned char FILE_APPENDER_CFG = 0x02;
	constexpr unsigned char TCP_APPENDER_CFG = 0x04;
	constexpr unsigned char UDP_APPENDER_CFG = 0x08;

	class appender_config {
		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, appender_config const &obj);

		friend appender_config tag_invoke(boost::json::value_to_tag<appender_config>, boost::json::value const &json);

	public:
		const console_appender_config *get_console_cfg() const {
			if (APPENDER_FLAGS & CONSOLE_APPENDER_CFG) {
				return &console_cfg;
			}
			return nullptr;
		}

		const file_appender_config *get_file_cfg() const {
			if (APPENDER_FLAGS & FILE_APPENDER_CFG) {
				return &file_cfg;
			}
			return nullptr;
		}

		const tcp_appender_config *get_tcp_cfg() const {
			if (APPENDER_FLAGS & TCP_APPENDER_CFG) {
				return &tcp_cfg;
			}
			return nullptr;
		}

		const udp_appender_config *get_udp_cfg() const {
			if (APPENDER_FLAGS & UDP_APPENDER_CFG) {
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

	class layout_config {
	public:
		layout_config();

		layout_config(const layout_config &other);

		layout_config(layout_config &&other) noexcept;

		layout_config &operator=(const layout_config &other);

		layout_config &operator=(layout_config &&other) noexcept;

		/**
		 * @brief Get the logger name
		 * @return The logger name
		 */
		[[nodiscard]] std::string get_logger_name() const;

		/* Get the logger level */
		[[nodiscard]] log_level get_logger_level() const;

		/* Get the layout flags */
		[[nodiscard]] unsigned char get_layout_flag() const;

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, layout_config const &obj);

		friend layout_config tag_invoke(boost::json::value_to_tag<layout_config>, boost::json::value const &json);

	private:
		/* Logger name */
		std::string name;
		/* Logger level */
		log_level level;
		/* Layout flag */
		unsigned char layout_flag{};
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, layout_config const &obj);

	layout_config tag_invoke(boost::json::value_to_tag<layout_config>, boost::json::value const &json);

	class log4cpp_config {
	public:
		/**
		 * @brief Load the configuration from a JSON file
		 * @param json_file The JSON file
		 * @return The configuration
		 */
		[[nodiscard]]
		static log4cpp_config load_config(const std::string &json_file);

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, log4cpp_config const &obj);

		friend log4cpp_config tag_invoke(boost::json::value_to_tag<log4cpp_config>, boost::json::value const &json);

		/**
		 * @brief Serialize the configuration to a JSON string
		 * @param obj The configuration
		 * @return The JSON string
		 */
		static std::string serialize(const log4cpp_config &obj);

		log4cpp_config() = default;

		log4cpp_config(std::string _pattern, const appender_config &o, const std::vector<layout_config> &l,
		               layout_config root);

		log4cpp_config(const log4cpp_config &other);

		log4cpp_config(log4cpp_config &&other) noexcept;

		log4cpp_config &operator=(const log4cpp_config &other);

		log4cpp_config &operator=(log4cpp_config &&other) noexcept;

		const std::string &get_layout_pattern() const;

		const appender_config &get_appender() const;

		const std::vector<layout_config> &get_layouts() const;

		const layout_config &get_root_layout() const;

		friend class layout_manager;

	private:
		std::string layout_pattern; // layoutPattern
		appender_config appender{}; // appenders
		std::vector<layout_config> layouts; // layouts
		layout_config root_layout; // rootLayout
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, log4cpp_config const &obj);

	log4cpp_config tag_invoke(boost::json::value_to_tag<log4cpp_config>, boost::json::value const &json);
}
