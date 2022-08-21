#ifndef LOG4CPP_LOGCONFIGER_H
#define LOG4CPP_LOGCONFIGER_H

#include <yaml-cpp/yaml.h>

#include "../include/log4cpp.hpp"

class Log4CppConfiger
{
public:
	Outputter *consoleOutputter;
	Outputter *fileOutputter;
	std::vector<Logger> loggers;
	RootLogger rootLogger;
public:
	Log4CppConfiger();

	~Log4CppConfiger();

	bool loadYamlConfig(const std::string &yamlFile);

private:
	void reset();
};


#endif //LOG4CPP_LOGCONFIGER_H
