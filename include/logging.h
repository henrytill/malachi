#pragma once

#include <format>
#include <iostream>
#include <string_view>

namespace malachi::logging
{

extern bool debug_enabled;

inline void log_info(std::string_view msg)
{
    std::cout << std::format("[INFO] {}\n", msg);
}

inline void log_error(std::string_view msg)
{
    std::cerr << std::format("[ERROR] {}\n", msg);
}

inline void log_debug(std::string_view msg)
{
    if (debug_enabled)
        std::cout << std::format("[DEBUG] {}\n", msg);
}

template <typename... Args>
void info(std::format_string<Args...> fmt, Args &&...args)
{
    log_info(std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
void error(std::format_string<Args...> fmt, Args &&...args)
{
    log_error(std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
void debug(std::format_string<Args...> fmt, Args &&...args)
{
    log_debug(std::format(fmt, std::forward<Args>(args)...));
}

} // namespace malachi::logging
