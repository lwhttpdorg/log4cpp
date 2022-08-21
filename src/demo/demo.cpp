
#include "log4cpp.hpp"

int main()
{
	LoggerManager::setYamlFilePath("./log4cpp.yml");
	Logger logger = LoggerManager::getLogger("main");
	logger.trace("This is a trace: %s:%d", __func__, __LINE__);
	logger.info("This is a info: %s:%d", __func__, __LINE__);
	logger.debug("This is a debug: %s:%d", __func__, __LINE__);
	logger.error("This is a error: %s:%d", __func__, __LINE__);
	logger.fatal("This is a fatal: %s:%d", __func__, __LINE__);

	logger = LoggerManager::getLogger("test");
	logger.trace("This is a trace: %s:%d", __func__, __LINE__);
	logger.info("This is a info: %s:%d", __func__, __LINE__);
	logger.debug("This is a debug: %s:%d", __func__, __LINE__);
	logger.error("This is a error: %s:%d", __func__, __LINE__);
	logger.fatal("This is a fatal: %s:%d", __func__, __LINE__);
	return 0;
}
