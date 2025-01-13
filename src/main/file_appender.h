#pragma once

#include <boost/json.hpp>

#include "log_appender.h"
#include "log_lock.h"

namespace log4cpp {
	class file_appender;

	class file_appender_config {
	public:
		/**
		 * @brief Get a file_appender instance with the given configuration
		 * @param config: The configuration of the file_appender
		 * @return File appender instance
		 */
		static std::shared_ptr<file_appender> get_instance(const file_appender_config &config);

		[[nodiscard]] std::string get_file_path() const {
			return file_path;
		}

		void set_file_path(const std::string &path) {
			this->file_path = path;
		}

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, file_appender_config const &obj);

		friend file_appender_config tag_invoke(boost::json::value_to_tag<file_appender_config>,
											   boost::json::value const &json);

	private:
		std::string file_path{};
		static log_lock instance_lock;
		static std::shared_ptr<file_appender> instance;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, file_appender_config const &obj);

	file_appender_config tag_invoke(boost::json::value_to_tag<file_appender_config>, boost::json::value const &json);

	class file_appender: public log_appender {
	public:
		class builder {
		public:
			/**
			 * @brief Set the file path for file appender
			 * @param file: The file path
			 * @return The builder
			 */
			builder &set_file(const std::string &file);

			/**
			 * @brief Build the file appender instance
			 * @return The file appender instance
			 */
			std::shared_ptr<file_appender> build();

			/**
			 * @brief Create a builder
			 * @return The builder
			 */
			static builder new_builder();

		private:
			builder() = default;

			file_appender_config config{};
			std::shared_ptr<file_appender> instance{nullptr};
		};

		file_appender(const file_appender &other) = delete;

		file_appender(file_appender &&other) = delete;

		file_appender &operator=(const file_appender &other) = delete;

		file_appender &operator=(file_appender &&other) = delete;

		void log(const char *msg, size_t msg_len) override;

		~file_appender() override;

	private:
		file_appender() = default;

		/* The fd of the log file */
		int fd{-1};
		log_lock lock;
	};
}
