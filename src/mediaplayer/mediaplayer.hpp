#pragma once

#include "mpv/client.h"
#include "mpv/render_gl.h"
#include "raylib.h"

#include <string>

namespace Rayplayer
{
struct MediaProperties
{
    int64_t videoWidth;
    int64_t videoHeight;
    std::string videoCodec;
};

class MediaPlayer final
{
  public:
    MediaPlayer() = default;
    ~MediaPlayer();

    MediaPlayer(MediaPlayer &&other) noexcept            = delete;
    MediaPlayer &operator=(MediaPlayer &&other) noexcept = delete;

    MediaPlayer(const MediaPlayer &)                     = delete;
    MediaPlayer &operator=(const MediaPlayer &)          = delete;

    void init();

    [[nodiscard]] const RenderTexture2D &texture() const;

    [[nodiscard]] const MediaProperties &mediaProps() const;

    [[nodiscard]] bool isPaused();

    void loadMedia(const char *file);
    void play();
    void pause();
    void seek(double secondsDelta);
    void volume(double valueDelta);

    void update();

  private:
    mpv_handle *m_mpvHandle{};
    mpv_render_context *m_mpvRenderCtx{};

    RenderTexture2D m_targetTexture{};

    MediaProperties m_mediaProps{};
};
}