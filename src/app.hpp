#pragma once

#include "ui/page.hpp"

#include <memory>
#include <unordered_map>

namespace Rayplayer
{
namespace details
{
class Application final
{
  public:
    Application();
    ~Application()                                       = default;

    Application(Application &&other) noexcept            = delete;
    Application &operator=(Application &&other) noexcept = delete;

    Application(const Application &)                     = delete;
    Application &operator=(const Application &)          = delete;

    void update();

  private:
    std::unordered_map<uint8_t, std::unique_ptr<Page>> m_pages;
    uint8_t m_nextId{1};
    uint8_t m_currentId{0};
};
}

int run();
}