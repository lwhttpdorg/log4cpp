#include <iostream>
#include <fstream>
#include <filesystem>
#include <utility>

#include "log4cpp_config.h"
#include "console_output.h"
#include "file_output.h"


using namespace log4cpp;

bool valid_output(const std::string &name) {
	bool valid_output = true;
	if ((name != CONSOLE_OUTPUT_NAME) && (name != FILE_OUTPUT_NAME) && (name != TCP_OUTPUT_NAME) &&
	    (name != UDP_OUTPUT_NAME)) {
		valid_output = false;
	}
	return valid_output;
}

void log4cpp::tag_invoke(boost::json::value_from_tag, boost::json::value &json, const output_config &obj) {
	json = boost::json::object{};
	if (obj.OUT_FLAGS&CONSOLE_OUT_CFG) {
		json.at(CONSOLE_OUTPUT_NAME) = boost::json::value_from(obj.console_cfg);
	}
	if (obj.OUT_FLAGS&FILE_OUT_CFG) {
		json.at(FILE_OUTPUT_NAME) = boost::json::value_from(obj.file_cfg);
	}
}

output_config log4cpp::tag_invoke(boost::json::value_to_tag<output_config>, boost::json::value const &json) {
	output_config output_cfg{};
	if (json.as_object().contains(CONSOLE_OUTPUT_NAME)) {
		output_cfg.console_cfg = boost::json::value_to<console_output_config>(json.at(CONSOLE_OUTPUT_NAME));
		output_cfg.OUT_FLAGS |= CONSOLE_OUT_CFG;
	}
	if (json.as_object().contains(FILE_OUTPUT_NAME)) {
		output_cfg.file_cfg = boost::json::value_to<file_output_config>(json.at(FILE_OUTPUT_NAME));
		output_cfg.OUT_FLAGS |= FILE_OUT_CFG;
	}
	return output_cfg;
}

logger_config::logger_config() {
	this->level = log_level::WARN;
}

logger_config::logger_config(const logger_config &other) {
	this->name = other.name;
	this->level = other.level;
	this->_outputs = other._outputs;
}

logger_config::logger_config(logger_config &&other) noexcept {
	this->name = std::move(other.name);
	this->level = other.level;
	this->_outputs = other._outputs;
}

logger_config &logger_config::operator=(const logger_config &other) {
	if (this != &other) {
		this->name = other.name;
		this->level = other.level;
		this->_outputs = other._outputs;
	}
	return *this;
}

logger_config &logger_config::operator=(logger_config &&other) noexcept {
	if (this != &other) {
		this->name = std::move(other.name);
		this->level = other.level;
		this->_outputs = other._outputs;
	}
	return *this;
}

std::string logger_config::get_logger_name() const {
	return this->name;
}

void logger_config::set_logger_name(const std::string &_name) {
	if (!valid_output(_name)) {
		throw std::invalid_argument("Unknown logOutPuts: " + _name);
	}
	logger_config::name = _name;
}

log_level logger_config::get_logger_level() const {
	return this->level;
}

void logger_config::set_logger_level(log_level _level) {
	logger_config::level = _level;
}

unsigned char logger_config::get_outputs() const {
	return _outputs;
}

logger_config log4cpp::tag_invoke(boost::json::value_to_tag<logger_config>, boost::json::value const &json) {
	logger_config obj;
	if (json.as_object().contains("name")) {
		obj.name = boost::json::value_to<std::string>(json.at("name"));
	}
	else {
		obj.name = "root";
	}
	obj.level = log4cpp::from_string(boost::json::value_to<std::string>(json.at("logLevel")));
	std::vector<std::string> outputs = boost::json::value_to<std::vector<std::string>>(json.at("logOutPuts"));
	for (auto &output:outputs) {
		if (output == CONSOLE_OUTPUT_NAME) {
			obj._outputs |= CONSOLE_OUT_CFG;
		}
		else if (output == FILE_OUTPUT_NAME) {
			obj._outputs |= FILE_OUT_CFG;
		}
		else if (output == TCP_OUTPUT_NAME) {
			obj._outputs |= TCP_OUT_CFG;
		}
		else if (output == UDP_OUTPUT_NAME) {
			obj._outputs |= UDP_OUT_CFG;
		}
	}
	return obj;
}

void log4cpp::tag_invoke(boost::json::value_from_tag, boost::json::value &json, const logger_config &obj) {
	std::vector<std::string> outputs;
	if (obj._outputs&CONSOLE_OUT_CFG) {
		outputs.emplace_back(CONSOLE_OUTPUT_NAME);
	}
	if (obj._outputs&FILE_OUT_CFG) {
		outputs.emplace_back(FILE_OUTPUT_NAME);
	}
	if (obj._outputs&TCP_OUT_CFG) {
		outputs.emplace_back(TCP_OUTPUT_NAME);
	}
	if (obj._outputs&UDP_OUT_CFG) {
		outputs.emplace_back(UDP_OUTPUT_NAME);
	}

	json = boost::json::object{{"name",       obj.name},
	                           {"logLevel",   log4cpp::to_string(obj.level)},
	                           {"logOutPuts", boost::json::value_from(outputs)}};
}

void log4cpp::tag_invoke(boost::json::value_from_tag, boost::json::value &json, const log4cpp_config &obj) {
	json = boost::json::object{{"pattern",    obj.pattern},
	                           {"logOutPut",  boost::json::value_from(obj.output)},
	                           {"loggers",    boost::json::value_from(obj.loggers)},
	                           {"rootLogger", boost::json::value_from(obj.root_logger)}};
}

log4cpp_config log4cpp::tag_invoke(boost::json::value_to_tag<log4cpp_config>, boost::json::value const &json) {
	std::string pattern = boost::json::value_to<std::string>(json.at("pattern"));
	output_config outputs = boost::json::value_to<output_config>(json.at("logOutPut"));
	std::vector<logger_config> loggers = boost::json::value_to<std::vector<logger_config>>(json.at("loggers"));
	logger_config root = boost::json::value_to<logger_config>(json.at("rootLogger"));
	return log4cpp_config{pattern, outputs, loggers, root};
}

log4cpp_config log4cpp_config::load_config(const std::string &json_file) {
	std::ifstream ifs;
	ifs.open(json_file, std::ios::in);
	if (!ifs.is_open()) {
		std::cerr << "JSON file opening failed" << std::endl;
		throw std::filesystem::filesystem_error("JSON file " + json_file + " opening failed",
		                                        std::make_error_code(std::io_errc::stream));
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
		size_t read_size = ifs.gcount();
		if (read_size > 0) {
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
		throw std::invalid_argument("JSON parse failed! " + error_code.message());
	}
	sparser.finish();
	boost::json::value json_value = sparser.release();
	return boost::json::value_to<log4cpp_config>(json_value);
}

std::string log4cpp::serialize(const log4cpp_config &obj) {
	// 将对象序列化为JSON
	boost::json::value json = boost::json::value_from(obj);
	// 将JSON反序列化
	return boost::json::serialize(json);
}

log4cpp_config::log4cpp_config(std::string _pattern, output_config &o, const std::vector<logger_config> &l,
                               logger_config root) {
	this->pattern = std::move(_pattern);
	this->output = o;
	this->loggers = l;
	this->root_logger = std::move(root);
}


log4cpp_config::log4cpp_config(const log4cpp_config &other) {
	this->pattern = other.pattern;
	this->output = other.output;
	this->loggers = other.loggers;
	this->root_logger = other.root_logger;
}

log4cpp_config::log4cpp_config(log4cpp_config &&other) noexcept {
	this->pattern = std::move(other.pattern);
	this->output = std::move(other.output);
	this->loggers = std::move(other.loggers);
	this->root_logger = std::move(other.root_logger);
}

log4cpp_config &log4cpp_config::operator=(const log4cpp_config &other) {
	if (this != &other) {
		this->pattern = other.pattern;
		this->output = other.output;
		this->loggers = other.loggers;
		this->root_logger = other.root_logger;
	}
	return *this;
}

log4cpp_config &log4cpp_config::operator=(log4cpp_config &&other) noexcept {
	if (this != &other) {
		this->pattern = std::move(other.pattern);
		this->output = std::move(other.output);
		this->loggers = std::move(other.loggers);
		this->root_logger = std::move(other.root_logger);
	}
	return *this;
}
