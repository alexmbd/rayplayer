#pragma once

namespace Rayplayer
{
class Page
{
  public:
    Page()                                 = default;
    virtual ~Page()                        = default;

    Page(Page &&other) noexcept            = default;
    Page &operator=(Page &&other) noexcept = default;

    Page(const Page &)                     = default;
    Page &operator=(const Page &)          = default;

    virtual void update()                  = 0;
    virtual void draw()                    = 0;
};
}