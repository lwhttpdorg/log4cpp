#include <pthread.h>

#include "../include/log4cpp.hpp"

void *thread_routine(void *args)
{
	Logger logger = LoggerManager::getLogger("test");
	logger.trace("This is a trace: %s:%d", __func__, __LINE__);
	logger.info("This is a info: %s:%d", __func__, __LINE__);
	logger.debug("This is a debug: %s:%d", __func__, __LINE__);
	logger.error("This is a error: %s:%d", __func__, __LINE__);
	logger.fatal("This is a fatal: %s:%d", __func__, __LINE__);
	return nullptr;
}

int main()
{
	LoggerManager::setYamlFilePath("log4cpp.yml");
	Logger logger = LoggerManager::getLogger("main");
	logger.trace("This is a trace: %s:%d", __func__, __LINE__);
	logger.info("This is a info: %s:%d", __func__, __LINE__);
	logger.debug("This is a debug: %s:%d", __func__, __LINE__);
	logger.warn("This is a warning: %s:%d", __func__, __LINE__);
	logger.error("This is a error: %s:%d", __func__, __LINE__);
	logger.fatal("This is a fatal: %s:%d", __func__, __LINE__);

	pthread_t th;
	pthread_create(&th, nullptr, thread_routine, nullptr);
	pthread_join(th, nullptr);

	return 0;
}
