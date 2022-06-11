#include <thread>

#include "log4cpp.hpp"

void func_a();

void func_b();

int main()
{
	logger main_logger("./demo.log", log_level::TRACE);

	std::thread t1(func_a);
	std::thread t2(func_b);
	pthread_setname_np(pthread_self(), "thread main");
	for (int i = 0; i < 100; ++i)
	{
		main_logger.log_warn("This is a warning...");
	}
	t1.join();
	t2.join();
	return 0;
}

void func_a()
{
	logger child_logger("./demo.log", log_level::TRACE);
	pthread_setname_np(pthread_self(), "thread a");
	for (int i = 0; i < 100; ++i)
	{
		child_logger.log_fatal("This is a fatal...");
	}
}

void func_b()
{
	logger child_logger("./demo.log", log_level::TRACE);
	pthread_setname_np(pthread_self(), "thread b");
	child_logger.set_prefix("thread b");
	for (int i = 0; i < 100; ++i)
	{
		child_logger.log(log_level::ERROR, "This is an error...");
	}
}