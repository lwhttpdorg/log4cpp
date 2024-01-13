#pragma once

#include "main/log_output.h"

namespace log4cpp
{
    class file_output: public log_output
    {
    public:
    public:
        class builder
        {
        public:
            builder &set_file(const std::string &file);

            builder &set_async(bool async);

            builder &set_append(bool append);

            file_output *build();

            static builder new_builder();

        private:
            file_output *instance{nullptr};
        };


        file_output(const file_output &other) = delete;

        file_output(file_output &&other) noexcept;

        file_output &operator=(const file_output &other) = delete;

        file_output &operator=(file_output &&other) noexcept;

        void log(log_level level, const char *__restrict fmt, va_list args) override;

        void log(log_level level, const char *__restrict fmt, ...) override;

        ~file_output() override;

    private:
        file_output() = default;

    private:
        int fd{-1};
    };

    class file_output_config
    {
    public:
        static file_output *get_instance(const file_output_config &self);

        friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, file_output_config const &obj);

        friend file_output_config
        tag_invoke(boost::json::value_to_tag<file_output_config>, boost::json::value const &json);

    private:
        std::string file_path;
        bool async{false};
        bool append{false};
        static file_output *instance;
        static log_lock instance_lock;
    };

    void tag_invoke(boost::json::value_from_tag, boost::json::value &json, file_output_config const &obj);

    file_output_config tag_invoke(boost::json::value_to_tag<file_output_config>, boost::json::value const &json);
}