#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#endif

#include <filesystem>
#include <fstream>
#include <utility>

#include "logger_pattern.h"
#include "log4cpp_config.h"
#include "log_utils.h"

using namespace log4cpp;

bool valid_appender(const std::string &name) {
	bool valid = true;
	if ((name != CONSOLE_APPENDER_NAME) && (name != FILE_APPENDER_NAME) && (name != TCP_APPENDER_NAME)
		&& (name != UDP_APPENDER_NAME)) {
		valid = false;
	}
	return valid;
}

void log4cpp::tag_invoke(boost::json::value_from_tag, boost::json::value &json, const appender_config &obj) {
	boost::json::object json_obj = boost::json::object{};
	if (obj.APPENDER_FLAGS & CONSOLE_APPENDER_FLAG) {
		json_obj[CONSOLE_APPENDER_NAME] = boost::json::value_from(obj.console_cfg);
	}
	if (obj.APPENDER_FLAGS & FILE_APPENDER_FLAG) {
		json_obj[FILE_APPENDER_NAME] = boost::json::value_from(obj.file_cfg);
	}
	if (obj.APPENDER_FLAGS & TCP_APPENDER_FLAG) {
		json_obj[TCP_APPENDER_NAME] = boost::json::value_from(obj.tcp_cfg);
	}
	if (obj.APPENDER_FLAGS & UDP_APPENDER_FLAG) {
		json_obj[UDP_APPENDER_NAME] = boost::json::value_from(obj.udp_cfg);
	}
	json = json_obj;
}

appender_config log4cpp::tag_invoke(boost::json::value_to_tag<appender_config>, boost::json::value const &json) {
	appender_config append_cfg{};
	auto json_obj = json.as_object();
	if (json_obj.contains(CONSOLE_APPENDER_NAME)) {
		append_cfg.console_cfg = boost::json::value_to<console_appender_config>(json_obj.at(CONSOLE_APPENDER_NAME));
		append_cfg.APPENDER_FLAGS |= CONSOLE_APPENDER_FLAG;
	}
	if (json_obj.contains(FILE_APPENDER_NAME)) {
		append_cfg.file_cfg = boost::json::value_to<file_appender_config>(json_obj.at(FILE_APPENDER_NAME));
		append_cfg.APPENDER_FLAGS |= FILE_APPENDER_FLAG;
	}
	if (json_obj.contains(TCP_APPENDER_NAME)) {
		append_cfg.tcp_cfg = boost::json::value_to<tcp_appender_config>(json_obj.at(TCP_APPENDER_NAME));
		append_cfg.APPENDER_FLAGS |= TCP_APPENDER_FLAG;
	}
	if (json_obj.contains(UDP_APPENDER_NAME)) {
		append_cfg.udp_cfg = boost::json::value_to<udp_appender_config>(json_obj.at(UDP_APPENDER_NAME));
		append_cfg.APPENDER_FLAGS |= UDP_APPENDER_FLAG;
	}
	return append_cfg;
}

logger_config::logger_config() {
	this->level = log_level::WARN;
}

logger_config::logger_config(const logger_config &other) {
	this->name = other.name;
	this->level = other.level;
	this->logger_flag = other.logger_flag;
}

logger_config::logger_config(logger_config &&other) noexcept {
	this->name = std::move(other.name);
	this->level = other.level;
	this->logger_flag = other.logger_flag;
}

logger_config &logger_config::operator=(const logger_config &other) {
	if (this != &other) {
		this->name = other.name;
		this->level = other.level;
		this->logger_flag = other.logger_flag;
	}
	return *this;
}

logger_config &logger_config::operator=(logger_config &&other) noexcept {
	if (this != &other) {
		this->name = std::move(other.name);
		this->level = other.level;
		this->logger_flag = other.logger_flag;
	}
	return *this;
}

std::string logger_config::get_logger_name() const {
	return this->name;
}

log_level logger_config::get_logger_level() const {
	return this->level;
}

unsigned char logger_config::get_logger_flag() const {
	return logger_flag;
}

logger_config log4cpp::tag_invoke(boost::json::value_to_tag<logger_config>, boost::json::value const &json) {
	logger_config obj;
	auto json_obj = json.as_object();
	if (json_obj.contains("name")) {
		obj.name = boost::json::value_to<std::string>(json_obj.at("name"));
	}
	else {
		obj.name = "root";
	}
	if (!json_obj.contains("log_level")) {
		throw std::invalid_argument("Malformed JSON configuration file: \"log_level\" is mandatory");
	}
	if (!json_obj.contains("appenders")) {
		throw std::invalid_argument("Malformed JSON configuration file: \"appenders\" is mandatory");
	}
	obj.level = log4cpp::from_string(boost::json::value_to<std::string>(json_obj.at("log_level")));
	std::vector<std::string> appenders = boost::json::value_to<std::vector<std::string>>(json_obj.at("appenders"));
	for (auto &appender: appenders) {
		if (!valid_appender(appender)) {
			throw std::invalid_argument("Malformed JSON configuration file: invalid loggers::appenders \"" + appender
										+ "\"");
		}
		if (appender == CONSOLE_APPENDER_NAME) {
			obj.logger_flag |= CONSOLE_APPENDER_FLAG;
		}
		else if (appender == FILE_APPENDER_NAME) {
			obj.logger_flag |= FILE_APPENDER_FLAG;
		}
		else if (appender == TCP_APPENDER_NAME) {
			obj.logger_flag |= TCP_APPENDER_FLAG;
		}
		else if (appender == UDP_APPENDER_NAME) {
			obj.logger_flag |= UDP_APPENDER_FLAG;
		}
	}
	return obj;
}

void log4cpp::tag_invoke(boost::json::value_from_tag, boost::json::value &json, const logger_config &obj) {
	std::vector<std::string> appenders;
	if (obj.logger_flag & CONSOLE_APPENDER_FLAG) {
		appenders.emplace_back(CONSOLE_APPENDER_NAME);
	}
	if (obj.logger_flag & FILE_APPENDER_FLAG) {
		appenders.emplace_back(FILE_APPENDER_NAME);
	}
	if (obj.logger_flag & TCP_APPENDER_FLAG) {
		appenders.emplace_back(TCP_APPENDER_NAME);
	}
	if (obj.logger_flag & UDP_APPENDER_FLAG) {
		appenders.emplace_back(UDP_APPENDER_NAME);
	}

	boost::json::object json_obj = boost::json::object{};
	if ("root" != obj.name) {
		json_obj["name"] = obj.name;
	}
	json_obj["log_level"] = to_string(obj.level);
	json_obj["appenders"] = boost::json::value_from(appenders);
	json = json_obj;
}

void log4cpp::tag_invoke(boost::json::value_from_tag, boost::json::value &json, const log4cpp_config &obj) {
	json = boost::json::object{{"logger_pattern", obj.logger_pattern},
							   {"appenders", boost::json::value_from(obj.appender)},
							   {"loggers", boost::json::value_from(obj.loggers)},
							   {"root_logger", boost::json::value_from(obj.root_logger)}};
}

log4cpp_config log4cpp::tag_invoke(boost::json::value_to_tag<log4cpp_config>, boost::json::value const &json) {
	auto json_obj = json.as_object();
	std::string pattern;
	if (json_obj.contains("logger_pattern")) {
		pattern = boost::json::value_to<std::string>(json_obj.at("logger_pattern"));
		logger_pattern::set_pattern(pattern);
	}
	if (!json_obj.contains("appenders")) {
		throw std::invalid_argument("Malformed JSON configuration file: \"appender\" is mandatory");
	}
	std::vector<logger_config> loggers;
	if (json_obj.contains("loggers")) {
		loggers = boost::json::value_to<std::vector<logger_config>>(json_obj.at("loggers"));
	}
	if (!json_obj.contains("root_logger")) {
		throw std::invalid_argument("Malformed JSON configuration file: \"root_logger\" is mandatory");
	}
	appender_config appenders = boost::json::value_to<appender_config>(json_obj.at("appenders"));
	const logger_config root = boost::json::value_to<logger_config>(json_obj.at("root_logger"));
	return log4cpp_config{pattern, appenders, loggers, root};
}

log4cpp_config log4cpp_config::load_config(const std::string &json_file) {
	std::ifstream ifs;
	ifs.open(json_file, std::ios::in);
	if (!ifs.is_open()) {
		std::filesystem::path current_path = std::filesystem::current_path();
		char errbuf[1024];
		log4c_scnprintf(errbuf, sizeof(errbuf), "%s:%u %s, can not open the file %s%s", __FILE__, __LINE__,
						__FUNCTION__, current_path.c_str(), json_file.c_str() + 1);
		throw std::filesystem::filesystem_error(errbuf, std::make_error_code(std::io_errc::stream));
	}
	std::string a_string;
	boost::system::error_code error_code;
	boost::json::parse_options parse_options{};
	parse_options.allow_comments = true;
	boost::json::storage_ptr sp{};
	boost::json::stream_parser sparser(sp, parse_options);
	while (ifs.good()) {
		char buffer[4096];
		ifs.read(buffer, sizeof(buffer) - 1);
		if (size_t read_size = ifs.gcount(); read_size > 0) {
			buffer[read_size] = '\0';
			a_string += std::string(buffer);
			sparser.write(a_string, error_code);
			if (error_code) {
				break;
			}
		}
	}
	ifs.close();
	if (error_code) {
		throw std::invalid_argument("Malformed JSON configuration file: JSON parse failed! " + error_code.message());
	}
	sparser.finish();
	boost::json::value json_value = sparser.release();
	return boost::json::value_to<log4cpp_config>(json_value);
}

std::string log4cpp_config::serialize(const log4cpp_config &obj) {
	// 将对象序列化为JSON
	const boost::json::value json = boost::json::value_from(obj);
	// 将JSON反序列化
	return boost::json::serialize(json);
}

log4cpp_config::log4cpp_config(std::string _pattern, const appender_config &o, const std::vector<logger_config> &l,
							   logger_config root) {
	this->logger_pattern = std::move(_pattern);
	logger_pattern::set_pattern(this->logger_pattern);
	this->appender = o;
	this->loggers = l;
	this->root_logger = std::move(root);
}

log4cpp_config::log4cpp_config(const log4cpp_config &other) {
	this->logger_pattern = other.logger_pattern;
	logger_pattern::set_pattern(this->logger_pattern);
	this->appender = other.appender;
	this->loggers = other.loggers;
	this->root_logger = other.root_logger;
}

log4cpp_config::log4cpp_config(log4cpp_config &&other) noexcept {
	this->logger_pattern = std::move(other.logger_pattern);
	logger_pattern::set_pattern(this->logger_pattern);
	this->appender = std::move(other.appender);
	this->loggers = std::move(other.loggers);
	this->root_logger = std::move(other.root_logger);
}

log4cpp_config &log4cpp_config::operator=(const log4cpp_config &other) {
	if (this != &other) {
		this->logger_pattern = other.logger_pattern;
		logger_pattern::set_pattern(this->logger_pattern);
		this->appender = other.appender;
		this->loggers = other.loggers;
		this->root_logger = other.root_logger;
	}
	return *this;
}

log4cpp_config &log4cpp_config::operator=(log4cpp_config &&other) noexcept {
	if (this != &other) {
		this->logger_pattern = std::move(other.logger_pattern);
		logger_pattern::set_pattern(this->logger_pattern);
		this->appender = std::move(other.appender);
		this->loggers = std::move(other.loggers);
		this->root_logger = std::move(other.root_logger);
	}
	return *this;
}

const std::string &log4cpp_config::get_logger_pattern() const {
	return logger_pattern;
}

const appender_config &log4cpp_config::get_appender() const {
	return appender;
}

const std::vector<logger_config> &log4cpp_config::get_loggers() const {
	return loggers;
}

const logger_config &log4cpp_config::get_root_logger() const {
	return root_logger;
}
