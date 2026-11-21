#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <vector>
#include <type_traits>
#include <utility>
#include <iterator>
#include <cstdint>
#include <typeinfo>

#if __has_include(<glm/glm.hpp>)
    #include <glm/glm.hpp>
    #include <glm/gtc/quaternion.hpp>
    #define VORTEX_HAS_GLM 1
#endif

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

namespace VortexLogDetail
{
    template<typename T, typename = void>
    struct IsStreamable : std::false_type {};

    template<typename T>
    struct IsStreamable<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<const T&>())>> : std::true_type {};

    template<typename T, typename = void>
    struct IsRange : std::false_type {};

    template<typename T>
    struct IsRange<T, std::void_t<decltype(std::begin(std::declval<const T&>())), decltype(std::end(std::declval<const T&>()))>> : std::true_type {};

    template<typename T, typename = void>
    struct HasToString : std::false_type {};

    template<typename T>
    struct HasToString<T, std::void_t<decltype(std::declval<const T&>().ToString())>> : std::true_type {};

    template<typename T>
    void AppendArg(std::ostream& os, const T& value);

    template<typename A, typename B>
    void AppendArg(std::ostream& os, const std::pair<A, B>& p);

    #ifdef VORTEX_HAS_GLM
        template<glm::length_t L, typename T, glm::qualifier Q>
        void AppendArg(std::ostream& os, const glm::vec<L, T, Q>& v);

        template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
        void AppendArg(std::ostream& os, const glm::mat<C, R, T, Q>& m);

        template<typename T, glm::qualifier Q>
        void AppendArg(std::ostream& os, const glm::qua<T, Q>& q);
    #endif

    template<typename T>
    void AppendArg(std::ostream& os, const T& value)
    {
        using D = std::decay_t<T>;

        if constexpr (std::is_same_v<D, bool>)
            os << (value ? "true" : "false");
        else if constexpr (std::is_same_v<D, int8_t> || std::is_same_v<D, uint8_t>)
            os << static_cast<int>(value);
        else if constexpr (HasToString<D>::value)
            os << value.ToString();
        else if constexpr (IsStreamable<D>::value)
            os << value;
        else if constexpr (IsRange<D>::value)
        {
            os << "[";
            bool first = true;
            for (const auto& e : value)
            {
                if (!first) os << ", ";
                first = false;
                AppendArg(os, e);
            }
            os << "]";
        }
        else if constexpr (std::is_enum_v<D>)
            os << static_cast<std::underlying_type_t<D>>(value);
        else
            os << "<unprintable:" << typeid(D).name() << ">";
    }

    template<typename A, typename B>
    void AppendArg(std::ostream& os, const std::pair<A, B>& p)
    {
        os << "(";
        AppendArg(os, p.first);
        os << ", ";
        AppendArg(os, p.second);
        os << ")";
    }

    #ifdef VORTEX_HAS_GLM
        template<glm::length_t L, typename T, glm::qualifier Q>
        void AppendArg(std::ostream& os, const glm::vec<L, T, Q>& v)
        {
            os << "(";
            for (glm::length_t i = 0; i < L; ++i)
            {
                if (i) os << ", ";
                AppendArg(os, v[i]);
            }
            os << ")";
        }

        template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
        void AppendArg(std::ostream& os, const glm::mat<C, R, T, Q>& m)
        {
            os << "[";
            for (glm::length_t r = 0; r < R; ++r)
            {
                if (r) os << ", ";
                os << "(";
                for (glm::length_t c = 0; c < C; ++c)
                {
                    if (c) os << ", ";
                    AppendArg(os, m[c][r]);
                }
                os << ")";
            }
            os << "]";
        }

        template<typename T, glm::qualifier Q>
        void AppendArg(std::ostream& os, const glm::qua<T, Q>& q)
        {
            os << "quat(" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ")";
        }
    #endif
}

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

    static void Print() { std::cout << LOG_RESET << std::endl; }

    template<typename T, typename... Args>
    static void Print(const T& first, const Args&... args)
    {
        VortexLogDetail::AppendArg(std::cout, first);
        Print(args...);
    }

    template<typename... Args>
    static void Log(LogLevel level, const char* color, const char* prefix, const Args&... args)
    {
        #ifdef _WIN32
            InitializeConsole();
        #endif

        const std::string timeStr = CurrentTime();

        std::ostringstream oss;
        (VortexLogDetail::AppendArg(oss, args), ...);
        const std::string messageStr = oss.str();

        {
            std::lock_guard<std::mutex> lock(s_LogMutex);
            s_LogBuffer.push_back({level, timeStr, prefix + messageStr});
        }

        std::cout << LOG_DIM << timeStr << LOG_RESET << " ";
        std::cout << color << LOG_BOLD << prefix << LOG_RESET;

        if (level != LogLevel::INFO)
            std::cout << color;

        std::cout << messageStr << LOG_RESET << std::endl;
    }
};

#define VORTEX_INFO(...)  VortexLog::Log(LogLevel::INFO,    LOG_COLOR_GREEN, " [INFO]    ", __VA_ARGS__)
#define VORTEX_WARN(...)  VortexLog::Log(LogLevel::WARNING, LOG_COLOR_WARN,  " [WARN]    ", __VA_ARGS__)
#define VORTEX_ERROR(...) VortexLog::Log(LogLevel::ERROR,   LOG_COLOR_RED,   " [ERROR]   ", __VA_ARGS__)
#define VORTEX_FATAL(...) VortexLog::Log(LogLevel::FATAL,   LOG_COLOR_CYAN,  " [FATAL]   ", __FILE__, ":", __LINE__, " - ", __VA_ARGS__)
