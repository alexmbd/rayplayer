#pragma once

#include <atomic>
#include <mutex>
#include <string>

namespace Rayplayer
{
namespace context
{
namespace details
{
class AppContext final
{
  public:
    static AppContext &get()
    {
        static AppContext instance;
        return instance;
    }

    AppContext(const AppContext &)            = delete;
    AppContext &operator=(const AppContext &) = delete;
    AppContext(AppContext &&)                 = delete;
    AppContext &operator=(AppContext &&)      = delete;

    inline void requestExit(const char *errorMessage)
    {
        {
            std::lock_guard<std::mutex> lock(m_errorMutex);
            if (m_lastError.empty()) { m_lastError = errorMessage; }
        }
        m_exitRequested.store(true, std::memory_order_release);
    }

    [[nodiscard]] inline bool shouldExit() const { return m_exitRequested.load(std::memory_order_acquire); }

    [[nodiscard]] inline std::string lastError() const
    {
        std::lock_guard<std::mutex> lock(m_errorMutex);
        return m_lastError;
    }

  private:
    AppContext()  = default;
    ~AppContext() = default;

    std::atomic<bool> m_exitRequested{false};
    mutable std::mutex m_errorMutex{};
    std::string m_lastError{};
};
}

inline void requestExit(const char *errorMessage) { details::AppContext::get().requestExit(errorMessage); }

[[nodiscard]] inline bool shouldExit() { return details::AppContext::get().shouldExit(); }

[[nodiscard]] inline std::string lastError() { return details::AppContext::get().lastError(); }
}
}