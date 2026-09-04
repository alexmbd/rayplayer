#pragma once

#include "mediaplayer/mediaplayer.hpp"
#include "page.hpp"

namespace Rayplayer
{
class MediaPlayerPage final : public Page
{
  public:
    MediaPlayerPage();
    ~MediaPlayerPage() override                                  = default;

    MediaPlayerPage(MediaPlayerPage &&other) noexcept            = delete;
    MediaPlayerPage &operator=(MediaPlayerPage &&other) noexcept = delete;

    MediaPlayerPage(const MediaPlayerPage &)                     = delete;
    MediaPlayerPage &operator=(const MediaPlayerPage &)          = delete;

    void update() override;
    void draw() override;

  private:
    MediaPlayer m_mediaPlayer;
};
}