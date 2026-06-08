#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <vector>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #ifdef APIENTRY
        #define VORTEX_SAVED_APIENTRY APIENTRY
        #undef APIENTRY
    #endif

    #include <windows.h>

    #ifdef APIENTRY
        #undef APIENTRY
    #endif

    #ifdef VORTEX_SAVED_APIENTRY
        #define APIENTRY VORTEX_SAVED_APIENTRY
        #undef VORTEX_SAVED_APIENTRY
    #endif

    #ifdef ERROR
        #undef ERROR
    #endif
#endif

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

struct LogEntry
{
    LogLevel level;
    std::string time;
    std::string message;
};

class VortexLog
{
    static std::string CurrentTime()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::tm buf{};
        #if defined(_WIN32)
            localtime_s(&buf, &in_time_t);
        #else
            localtime_r(&in_time_t, &buf);
        #endif

        std::stringstream ss;
        ss << std::put_time(&buf, "[%H:%M:%S]");
        
        return ss.str();
    }
    #ifdef _WIN32
        static void InitializeConsole()
        {        
                static bool initialized = false;
                if (!initialized)
                {
                    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
                    if (hOut != INVALID_HANDLE_VALUE)
                    {
                        DWORD dwMode = 0;
                        if (GetConsoleMode(hOut, &dwMode))
                        {
                            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                            SetConsoleMode(hOut, dwMode);
                        }
                    }

                    initialized = true;
                }
        }
    #endif
public:
    inline static std::vector<LogEntry> s_LogBuffer;
    inline static std::mutex s_LogMutex;

    static void Print()
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
        #ifdef _WIN32
            InitializeConsole();
        #endif

        std::string timeStr = CurrentTime();

        std::ostringstream oss;
        (oss << ... << std::forward<Args>(args));

        std::string messageStr = oss.str();

        {
            std::lock_guard<std::mutex> lock(s_LogMutex);
            s_LogBuffer.push_back({level, timeStr, prefix + messageStr});
        }

        std::cout << LOG_DIM << CurrentTime() << LOG_RESET << " ";        
        std::cout << color << LOG_BOLD << prefix << LOG_RESET;

        if (level == LogLevel::WARNING || level == LogLevel::ERROR || level == LogLevel::FATAL) 
        {
            std::cout << color;
        }

        std::cout << messageStr << LOG_RESET << std::endl;
    }
};

#define VORTEX_INFO(...)  VortexLog::Log(LogLevel::INFO,    LOG_COLOR_GREEN, " [INFO]    ", __VA_ARGS__)
#define VORTEX_WARN(...)  VortexLog::Log(LogLevel::WARNING, LOG_COLOR_WARN,  " [WARN]    ", __VA_ARGS__)
#define VORTEX_ERROR(...) VortexLog::Log(LogLevel::ERROR,   LOG_COLOR_RED,   " [ERROR]   ", __VA_ARGS__)
#define VORTEX_FATAL(...) VortexLog::Log(LogLevel::FATAL,   LOG_COLOR_CYAN,  " [FATAL]   ", __FILE__, ":", __LINE__, " - ", __VA_ARGS__)
