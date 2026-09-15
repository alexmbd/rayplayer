#pragma once

#include "mediaplayer/mediaplayer.hpp"
#include "page.hpp"
#include "widgets.hpp"

#include <string>
#include <unordered_map>

namespace Rayplayer
{
class MediaPlayerPage final : public Page
{
  public:
    MediaPlayerPage(Font *font);
    ~MediaPlayerPage() override;

    MediaPlayerPage(MediaPlayerPage &&other) noexcept            = delete;
    MediaPlayerPage &operator=(MediaPlayerPage &&other) noexcept = delete;

    MediaPlayerPage(const MediaPlayerPage &)                     = delete;
    MediaPlayerPage &operator=(const MediaPlayerPage &)          = delete;

    void update() override;
    void draw() override;

  private:
    struct MediaInfo
    {
        double duration{0.0};
        double currentTime{0.0};
    };

    Font *m_font; // Non-owning pointer to Font
    std::unordered_map<std::string, Texture2D> m_textures;

    MediaPlayer m_mediaPlayer;
    MediaInfo m_mediaInfo;

    Slider m_progressSlider;
    Button m_playBtn;
    Button m_volumeBtn;
    Slider m_volumeSlider;

    void computeLayout();
};
}