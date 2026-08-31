#pragma once

#include <print>

namespace Rayplayer
{
namespace logger
{
namespace details
{
class Log final
{
  public:
    enum class Level
    {
        Info,
        Warning,
        Error
    };

    static Log &get()
    {
        static Log instance;
        return instance;
    }

    Log(const Log &)            = delete;
    Log &operator=(const Log &) = delete;
    Log(Log &&)                 = delete;
    Log &operator=(Log &&)      = delete;

    void log(Level level, const std::string &message)
    {
        std::string logLevel = "";
        switch (level)
        {
        case Level::Info: logLevel = "Info"; break;
        case Level::Warning: logLevel = "Warning"; break;
        case Level::Error: logLevel = "Error"; break;
        }
        std::println("[Rayplayer {}] {}", logLevel, message);
    }

  private:
    Log()  = default;
    ~Log() = default;
};
}

template <typename... Args> inline void info(std::format_string<Args...> fmt, Args &&...args)
{ details::Log::get().log(details::Log::Level::Info, std::format(fmt, std::forward<Args>(args)...)); }

template <typename... Args> inline void warning(std::format_string<Args...> fmt, Args &&...args)
{ details::Log::get().log(details::Log::Level::Warning, std::format(fmt, std::forward<Args>(args)...)); }

template <typename... Args> inline void error(std::format_string<Args...> fmt, Args &&...args)
{ details::Log::get().log(details::Log::Level::Error, std::format(fmt, std::forward<Args>(args)...)); }
}
}