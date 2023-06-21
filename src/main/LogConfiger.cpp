#include <iostream>
#include "LogConfiger.h"

namespace YAML {
    template<>
    struct convert<LogLevel> {
        static Node encode(const LogLevel &level) {
            Node node;
            std::string value;
            switch (level) {
                case LogLevel::FATAL:
                    value = "fatal";
                    break;
                case LogLevel::ERROR:
                    value = "errno";
                    break;
                case LogLevel::WARN:
                    value = "warn";
                    break;
                case LogLevel::INFO:
                    value = "info";
                    break;
                case LogLevel::DEBUG:
                    value = "debug";
                    break;
                case LogLevel::TRACE:
                    value = "trace";
                    break;
            }
            node = value;
            return node;
        }

        static bool decode(const Node &node, LogLevel &level) {
            assert(node.Type() == YAML::NodeType::value::Scalar);
            auto value = node.as<std::string>();
            if (value == "fatal") {
                level = LogLevel::FATAL;
            } else if (value == "error") {
                level = LogLevel::ERROR;
            } else if (value == "warn") {
                level = LogLevel::WARN;
            } else if (value == "debug") {
                level = LogLevel::DEBUG;
            } else if (value == "info") {
                level = LogLevel::INFO;
            } else if (value == "trace") {
                level = LogLevel::TRACE;
            } else {
                return false;
            }
            return true;
        }
    };
}

namespace YAML {
    template<>
    struct convert<ConsoleOutputter> {
        static Node encode(const ConsoleOutputter &outputter) {
            Node node;
            node.push_back(outputter.logLevel);
            return node;
        }

        static bool decode(const Node &node, ConsoleOutputter &outputter) {
            assert(node.Type() == YAML::NodeType::value::Map);
            outputter.logLevel = node["logLevel"].as<LogLevel>();
            return node.Type() == YAML::NodeType::value::Map;
        }
    };
}

namespace YAML {
    template<>
    struct convert<FileOutputter> {
        static Node encode(const FileOutputter &outputter) {
            Node node;
            node.push_back(outputter.filePath);
            node.push_back(outputter.async);
            node.push_back(outputter.append);
            return node;
        }

        static bool decode(const Node &node, FileOutputter &outputter) {
            assert(node.Type() == YAML::NodeType::value::Map);
            outputter.filePath = node["filePath"].as<std::string>();
            outputter.async = node["async"].as<bool>();
            outputter.append = node["append"].as<bool>();
            return node.Type() == YAML::NodeType::value::Map;
        }
    };
}

namespace YAML {
    template<>
    struct convert<RootLogger> {
        static Node encode(const RootLogger &root) {
            Node node;
            node.push_back(root.pattern);
            node.push_back(root.logLevel);
            YAML::Node outputterNode;
            if (root.consoleOutputterEnabled) {
                outputterNode.push_back("consoleOutputter");
            }
            if (root.fileOutputterEnabled) {
                outputterNode.push_back("fileOutputter");
            }

            node.push_back(outputterNode);
            return node;
        }

        static bool decode(const Node &node, RootLogger &root) {
            assert(node.Type() == YAML::NodeType::value::Map);
            root.pattern = node["pattern"].as<std::string>();
            root.logLevel = node["logLevel"].as<LogLevel>();
            YAML::Node outputterNode = node["outputter"];
            if (outputterNode["consoleOutputter"]) {
                root.consoleOutputterEnabled = true;
            }
            if (outputterNode["fileOutputter"]) {
                root.fileOutputterEnabled = true;
            }
            return true;
        }
    };
}

namespace YAML {
    template<>
    struct convert<Logger> {
        static Node encode(const Logger &logger) {
            Node node;
            node.push_back(logger.name);
            node.push_back(logger.logLevel);
            YAML::Node outputterNode;
            if (logger.consoleOutputterEnabled) {
                outputterNode.push_back("consoleOutputter");
            }
            if (logger.fileOutputterEnabled) {
                outputterNode.push_back("fileOutputter");
            }
            node.push_back(outputterNode);
            return node;
        }

        static bool decode(const Node &node, Logger &logger) {
            assert(node.Type() == YAML::NodeType::value::Map);
            logger.name = node["name"].as<std::string>();
            logger.logLevel = node["logLevel"].as<LogLevel>();
            YAML::Node outputterNode = node["outputter"];
            auto outputters = outputterNode.as<std::vector<std::string>>();
            for (const auto &outputter: outputters) {
                if (outputter == "consoleOutputter") {
                    logger.consoleOutputterEnabled = true;
                }
                if (outputter == "fileOutputter") {
                    logger.fileOutputterEnabled = true;
                }
            }
            return true;
        }
    };
}

bool Log4CppConfiger::loadYamlConfig(const std::string &yamlFile) {
    YAML::Node yamlNode = YAML::LoadFile(yamlFile);
    YAML::Node log4cppNode = yamlNode["log4cpp"];
    YAML::Node outputtersNode = log4cppNode["outputters"];

    this->reset();

    if (outputtersNode["consoleOutputter"]) {
        YAML::Node logLevelNode = outputtersNode["consoleOutputter"]["logLevel"];
        LogLevel logLevel = logLevelNode.as<LogLevel>();
        this->consoleOutputter = new ConsoleOutputter(logLevel);
    }
    if (outputtersNode["fileOutputter"]) {
        std::string filePath;
        bool async = true;
        bool append = true;
        YAML::Node node = outputtersNode["fileOutputter"];
        for (auto it = node.begin(); it != node.end(); ++it) {
            std::string name = it->first.as<std::string>();
            YAML::Node value = it->second;
            if (name == "filePath") {
                filePath = value.as<std::string>();
            } else if (name == "async") {
                async = value.as<bool>();
            } else if (name == "append") {
                append = value.as<bool>();
            }
        }
        this->fileOutputter = new FileOutputter(filePath, async, append);
    }
    YAML::Node loggersNode = log4cppNode["loggers"];
    this->loggers = loggersNode.as<std::vector<Logger>>();
    YAML::Node rootNode = log4cppNode["root"];
    this->rootLogger = rootNode.as<RootLogger>();
    for (Logger &logger: this->loggers) {
        if (logger.consoleOutputterEnabled) {
            logger.consoleOutputter = this->consoleOutputter;
        }
        if (logger.fileOutputterEnabled) {
            logger.fileOutputter = this->fileOutputter;
        }
    }
    return true;
}

Log4CppConfiger::Log4CppConfiger() {
    consoleOutputter = nullptr;
    fileOutputter = nullptr;
}

Log4CppConfiger::~Log4CppConfiger() {
    delete consoleOutputter;
    delete fileOutputter;
}

void Log4CppConfiger::reset() {
    if (consoleOutputter != nullptr) {
        delete consoleOutputter;
        consoleOutputter = nullptr;
    }
    if (fileOutputter != nullptr) {
        delete fileOutputter;
        fileOutputter = nullptr;
    }

    if (!this->loggers.empty()) {
        this->loggers.clear();
    }

    this->rootLogger.pattern.clear();
    this->rootLogger.logLevel = LogLevel::ERROR;
    this->rootLogger.consoleOutputterEnabled = false;
    this->rootLogger.fileOutputterEnabled = false;
    if (this->rootLogger.consoleOutputter != nullptr) {
        delete this->rootLogger.consoleOutputter;
        this->rootLogger.consoleOutputter = nullptr;
    }
    if (this->rootLogger.fileOutputter != nullptr) {
        delete this->rootLogger.fileOutputter;
        this->rootLogger.fileOutputter = nullptr;
    }
}
