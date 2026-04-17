#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#define LOG_RESET       "\033[0m"
#define LOG_BOLD        "\033[1m"
#define LOG_DIM         "\033[90m"
#define LOG_COLOR_GREEN "\033[32m"
#define LOG_COLOR_WARN  "\033[38;5;214m"
#define LOG_COLOR_RED   "\033[31m"
#define LOG_COLOR_CYAN  "\033[36m"

enum class LogLevel
{
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class VortexLog
{
    static std::string CurrentTime()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "[%H:%M:%S]");
        
        return ss.str();
    }
public:
    static void print()
    {
        std::cout << LOG_RESET << std::endl;
    }

    template<typename T, typename... Args>
    static void Print(T&& first, Args&&... args)
    {
        std::cout << std::forward<T>(first);
        Print(std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void Log(LogLevel level, const char* color, const char* prefix, Args&&... args) 
    {
        std::cout << LOG_DIM << CurrentTime() << LOG_RESET << " ";
        
        std::cout << color << LOG_BOLD << prefix << LOG_RESET;

        if (level == LogLevel::WARNING || level == LogLevel::ERROR || level == LogLevel::FATAL) 
        {
            std::cout << color;
        }

        int dummy[] = { 0, ( (void)(std::cout << std::forward<Args>(args)), 0 ) ... };
        (void)dummy;

        std::cout << LOG_RESET << std::endl;
    }
};

#ifdef VORTEX_RELEASE
    #define VORTEX_INFO(...)  
    #define VORTEX_WARN(...)  
    #define VORTEX_ERROR(...) 
    #define VORTEX_FATAL(...) 
#else
    #define VORTEX_INFO(...)  VortexLog::Log(LogLevel::INFO,    LOG_COLOR_GREEN, " [INFO]    ", __VA_ARGS__)
    #define VORTEX_WARN(...)  VortexLog::Log(LogLevel::WARNING, LOG_COLOR_WARN,  " [WARN]    ", __VA_ARGS__)
    #define VORTEX_ERROR(...) VortexLog::Log(LogLevel::ERROR,   LOG_COLOR_RED,   " [ERROR]   ", __VA_ARGS__)
    #define VORTEX_FATAL(...) VortexLog::Log(LogLevel::FATAL,   LOG_COLOR_CYAN,  " [FATAL]   ", __FILE__, ":", __LINE__, " - ", __VA_ARGS__)
#endif
