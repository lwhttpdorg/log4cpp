#pragma once

#include <string>
#include <unordered_map>
#include <cassert>

#include <boost/json.hpp>
#include <list>
#include <vector>

namespace log4cpp
{
	enum class log_level
	{
		FATAL = 0, ERROR = 1, WARN = 2, INFO = 3, DEBUG = 4, TRACE = 5
	};

	std::string to_string(log_level level);

	log_level from_string(const std::string &s);

	class log_output;

	class output;

	class logger
	{
	public:
		logger();

		explicit logger(const std::string &log_name);

		logger(const std::string &log_name, log_level l, const std::vector<std::string> &out);

		logger(const logger &other);

		logger(logger &&other) noexcept;

		logger &operator=(const logger &other);

		logger &operator=(logger &&other) noexcept;

		void log(log_level _level, const char *__restrict fmt, va_list args);

		void fatal(const char *__restrict fmt, ...);

		void error(const char *__restrict fmt, ...);

		void warn(const char *__restrict fmt, ...);

		void info(const char *__restrict fmt, ...);

		void debug(const char *__restrict fmt, ...);

		void trace(const char *__restrict fmt, ...);

		virtual ~logger();

		//为了能够访问成员变量, 序列化和反序列化函数定义为友元
		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, logger const &obj);

		friend logger tag_invoke(boost::json::value_to_tag<logger>, boost::json::value const &json_value);

		friend class logger_builder;

		friend class log4cpp_config;

	private:
		std::string name;
		log_level level;
		output *outputs{nullptr};
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, logger const &obj);

	logger tag_invoke(boost::json::value_to_tag<logger>, boost::json::value const &json_value);

	/*********************** logger_manager ***********************/
	class log4cpp_config;

	class log_lock;

	class logger_manager
	{
	public:
		static void load_config(const std::string &json_filepath);

		static logger get_logger(const std::string &name);

	private:
		logger_manager() = default;

		logger build_logger(const std::string &name);

		class auto_load_config
		{
		public:
			auto_load_config();

			~auto_load_config();
		};

	private:
		static bool initialized;
		static log4cpp_config config;
		static auto_load_config init;
		static log_lock lock;
		static std::unordered_map<std::string, logger> loggers;
	};
}
