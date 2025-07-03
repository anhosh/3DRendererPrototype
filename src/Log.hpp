#pragma once

#include <print>

#define ENABLE_LOGGING 1

#if ENABLE_LOGGING
# define LOG_INFO(...) std::println(stdout, __VA_ARGS__)
# define LOG_ERROR(...) std::println(stderr, __VA_ARGS__)
#else
# define LOG_INFO(...)
# define LOG_ERROR(...)
#endif
