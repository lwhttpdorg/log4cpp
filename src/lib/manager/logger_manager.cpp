#include <appender/file_appender.hpp>
#include <appender/socket_appender.hpp>
#include <csignal>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sys/eventfd.h>

#include <nlohmann/json.hpp>

#include <log4cpp/log4cpp.hpp>
#include <log4cpp/logger.hpp>
#include <logger/core_logger.hpp>

#include "appender/console_appender.hpp"
#include "config/log4cpp.hpp"
#include "pattern/log_pattern.hpp"

constexpr const char *DEFAULT_CONFIG_FILE_PATH = "./log4cpp.json";

namespace log4cpp {

    enum EVENT_TYPE { EVT_HOT_RELOAD = 1, EVT_SHUTDOWN = 2 };

    std::once_flag logger_manager::init_flag;
    logger_manager logger_manager::instance;

    void supervisor::sigusr2_handle([[maybe_unused]] int sig_num) {
#ifdef _DEBUG
        printf("%s:%d, log4pp received hot reload trigger event\n", __func__, __LINE__);
#endif
        const logger_manager &logger_mgr = get_logger_manager();
        logger_mgr.notify_config_hot_reload();
    }

    bool supervisor::enable_config_hot_loading(int sig) {
        struct sigaction sa{};

        sa.sa_handler = sigusr2_handle;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        if (sigaction(sig, &sa, nullptr) == -1) {
            return false;
        }
        logger_manager &logger_mgr = get_logger_manager();
        logger_mgr.start_hot_reload_thread();
        return true;
    }

    logger_manager &supervisor::get_logger_manager() {
        return logger_manager::instance;
    }

    std::string supervisor::serialize(const config::log4cpp &cfg) {
        return config::log4cpp::serialize(cfg);
    }

    // ========================================
    // logger manager
    // ========================================

    logger_manager::logger_manager() {
        evt_fd = -1;
        evt_loop_run.store(false);
        config_file_path = DEFAULT_CONFIG_FILE_PATH;
        console_appender_ptr = nullptr;
        file_appender_ptr = nullptr;
        socket_appender_ptr = nullptr;
        root_logger = nullptr;
    }

    logger_manager::~logger_manager() {
        if (evt_loop_thread.joinable()) {
            evt_loop_run.store(false);
            constexpr uint64_t val = EVT_SHUTDOWN;
            (void)write(evt_fd, &val, sizeof(uint64_t));
            evt_loop_thread.join();
        }
        if (evt_fd != -1) {
            close(evt_fd);
        }
    }

    void logger_manager::auto_load_config() {
        std::filesystem::path fsp(config_file_path);

        if (exists(fsp)) {
            try {
                load_config(config_file_path);
                return;
            }
            catch (const std::exception &e) {
                printf("Failed to load the configuration file automatically, using default configuration. [%s]\n",
                       e.what());
            }
        }

        this->config = std::make_unique<config::log4cpp>();
        this->config->log_pattern = DEFAULT_LOG_PATTERN;
        this->config->appenders.console = config::console_appender{.out_stream = "stdout"};
        this->config->root_logger.appender_flag = static_cast<unsigned char>(config::APPENDER_TYPE::CONSOLE);
        this->config->root_logger.name = "root";
        this->config->root_logger.level = log_level::ERROR;
    }

    void logger_manager::load_config(const std::string &file_path) {
        if (-1 != access(file_path.c_str(), F_OK)) {
            std::ifstream ifs(file_path);
            if (!ifs.is_open()) {
                throw std::runtime_error("cannot open config file: " + file_path);
            }
            nlohmann::json j;
            ifs >> j;

            config = std::make_unique<config::log4cpp>(j.get<config::log4cpp>());
            config_file_path = file_path;
        }
        else {
            throw std::filesystem::filesystem_error("Config file " + file_path + " open failed!",
                                                    std::make_error_code(std::io_errc::stream));
        }
    }

    void logger_manager::event_loop() {
        set_thread_name("event_loop");
        uint64_t event;

        while (evt_loop_run.load()) {
            ssize_t s = read(evt_fd, &event, sizeof(uint64_t));
            if (s == sizeof(uint64_t)) {
                switch (event) {
                    case EVT_HOT_RELOAD:
                        hot_reload_config();
                        break;
                    case EVT_SHUTDOWN:
                        break;
                    default:
#ifdef _DEBUG
                        printf("%s:%d, received unknown event %lu\n", __func__, __LINE__, event);
#endif
                        break;
                }
            }
            else if (s == -1) {
                if (errno == EINTR) {
                    continue;
                }
                if (errno != EBADF) {
                    printf("%s: event fd read error! break event loop!\n", __func__);
                }
                break;
            }
        }
    }

    void logger_manager::notify_config_hot_reload() const {
        constexpr uint64_t val = EVT_HOT_RELOAD;
        (void)write(evt_fd, &val, sizeof(uint64_t));
    }

    void logger_manager::start_hot_reload_thread() {
        evt_loop_run.store(true);
        evt_fd = eventfd(0, 0);
        evt_loop_thread = std::thread(&logger_manager::event_loop, this);
    }

    void logger_manager::hot_reload_config() {
#ifdef _DEBUG
        printf("%s:%d, trigger hot reload config\n", __func__, __LINE__);
#endif
        std::unique_lock writer_lock(rw_lock);
        try {
            load_config(config_file_path);
        }
        catch (const std::exception &e) {
            printf("%s:%d, failed to reload config: %s\n", __func__, __LINE__, e.what());
            return;
        }
        set_log_pattern();
        build_appender();
        build_logger();
        build_root_logger();
        for (auto it = loggers.begin(); it != loggers.end();) {
            auto &proxy = it->second;
            if (proxy->hot_reload_flag_is_set()) {
                proxy->reset_hot_reload_flag();
                ++it;
            }
            else {
                /* TThe new configuration does not contain a logger with this name, and the missing proxy will be
                 * removed from the loggers. If the proxy is in use, it will not be invalidated because the reference
                 * count of the $\text{shared\_ptr}$ is not zero. However, the target object it proxies will be replaced
                 * by the default root logger, just as the next get_logger will.
                 */
                proxy->set_target(this->root_logger->get_target());
                it = loggers.erase(it);
            }
        }
    }

    std::shared_ptr<log::logger> logger_manager::get_logger(const std::string &name) {
        std::call_once(init_flag, [] {
            if (nullptr == instance.config) {
                instance.auto_load_config();
            }
            instance.set_log_pattern();
            instance.build_appender();
            instance.build_logger();
            instance.build_root_logger();
        });
        return instance.find_logger(name);
    }

    const config::log4cpp *logger_manager::get_config() const {
        return this->config.get();
    }

    std::shared_ptr<log::logger> logger_manager::find_logger(const std::string &name) {
        std::shared_lock reader_lock(rw_lock);
        if (loggers.find(name) == loggers.end()) {
            return root_logger;
        }
        return loggers.at(name);
    }

    void logger_manager::set_log_pattern() const {
        pattern::log_pattern::set_pattern(config->log_pattern);
    }

    void logger_manager::build_appender() {
        const config::log_appender &appender_cfg = config->appenders;
        std::shared_ptr<appender::log_appender> new_console_appender = nullptr;
        std::shared_ptr<appender::log_appender> new_file_appender = nullptr;
        std::shared_ptr<appender::log_appender> new_socket_appender = nullptr;
        if (appender_cfg.console.has_value()) {
            new_console_appender = std::make_shared<appender::console_appender>(appender_cfg.console.value());
        }
        if (appender_cfg.file.has_value()) {
            new_file_appender = std::make_shared<appender::file_appender>(appender_cfg.file.value());
        }
        if (appender_cfg.socket.has_value()) {
            new_socket_appender = std::make_shared<appender::socket_appender>(appender_cfg.socket.value());
        }

        std::unique_lock lock(appender_mtx);
        this->console_appender_ptr = new_console_appender;
        this->file_appender_ptr = new_file_appender;
        this->socket_appender_ptr = new_socket_appender;
    }

    void logger_manager::build_logger() {
        std::shared_ptr<appender::log_appender> temp_appenders[3];
        {
            std::shared_lock appender_lock(appender_mtx);
            temp_appenders[0] = this->console_appender_ptr;
            temp_appenders[1] = this->file_appender_ptr;
            temp_appenders[2] = this->socket_appender_ptr;
        }
        std::unique_lock logger_lock(this->logger_map_mtx);
        for (auto &lg: config->loggers) {
            std::shared_ptr<log::core_logger> new_logger = std::make_shared<log::core_logger>(lg.name, lg.level);
            if (lg.appender_flag & static_cast<unsigned char>(config::APPENDER_TYPE::CONSOLE)) {
                new_logger->add_appender(temp_appenders[0]);
            }
            if (lg.appender_flag & static_cast<unsigned char>(config::APPENDER_TYPE::FILE)) {
                new_logger->add_appender(temp_appenders[1]);
            }
            if (lg.appender_flag & static_cast<unsigned char>(config::APPENDER_TYPE::SOCKET)) {
                new_logger->add_appender(temp_appenders[2]);
            }
            std::shared_ptr<log::logger_proxy> proxy = this->loggers[lg.name];
            if (proxy == nullptr) {
                this->loggers[lg.name] = std::make_shared<log::logger_proxy>(new_logger);
            }
            else {
                proxy->set_hot_reload_flag();
                proxy->set_target(new_logger);
            }
        }
    }

    void logger_manager::build_root_logger() {
        std::shared_ptr<appender::log_appender> temp_appenders[3];
        {
            std::shared_lock appender_lock(this->appender_mtx);
            temp_appenders[0] = this->console_appender_ptr;
            temp_appenders[1] = this->file_appender_ptr;
            temp_appenders[2] = this->socket_appender_ptr;
        }
        std::unique_lock logger_lock(this->logger_map_mtx);
        const config::logger &cfg = this->config->root_logger;
        std::shared_ptr<log::core_logger> new_logger = std::make_shared<log::core_logger>(cfg.name, cfg.level);
        if (cfg.appender_flag & static_cast<unsigned char>(config::APPENDER_TYPE::CONSOLE)) {
            new_logger->add_appender(temp_appenders[0]);
        }
        if (cfg.appender_flag & static_cast<unsigned char>(config::APPENDER_TYPE::FILE)) {
            new_logger->add_appender(temp_appenders[1]);
        }
        if (cfg.appender_flag & static_cast<unsigned char>(config::APPENDER_TYPE::SOCKET)) {
            new_logger->add_appender(temp_appenders[2]);
        }

        if (nullptr == this->root_logger) {
            this->root_logger = std::make_shared<log::logger_proxy>(new_logger);
        }
        else {
            this->root_logger->set_target(new_logger);
        }
    }
}
