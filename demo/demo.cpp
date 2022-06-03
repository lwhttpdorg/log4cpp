#include <thread>

#include "log4cpp.hpp"

void func_a();

void func_b();

int main()
{
	logger main_logger("./demo.log", log_level::TRACE);
	main_logger.set_log_prefix("main");

	std::thread t1(func_a);
	std::thread t2(func_b);
	for (int i = 0; i < 100; ++i)
	{
		main_logger.log_warn("I am main thread %u", pthread_self());
	}
	t1.join();
	t2.join();
	return 0;
}

void func_a()
{
	logger child_logger("./demo.log", log_level::TRACE);
	child_logger.set_log_prefix("child");
	for (int i = 0; i < 100; ++i)
	{
		child_logger.log_fatal("I am child thread %u", pthread_self());
	}
}

void func_b()
{
	logger child_logger("./demo.log", log_level::TRACE);
	for (int i = 0; i < 100; ++i)
	{
		child_logger.log(log_level::ERROR, "I am child thread %u", pthread_self());
	}
}