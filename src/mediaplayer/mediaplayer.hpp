#pragma once

#include "mpv/client.h"
#include "mpv/render_gl.h"
#include "raylib.h"

#include <atomic>
#include <string>
#include <thread>

struct GLFWwindow;

namespace Rayplayer
{
// Media properties are things about the media that will not change (constants)
struct MediaProperties
{
    int64_t videoWidth;
    int64_t videoHeight;
    double duration;
    std::string videoCodec;
    std::string title;
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

    [[nodiscard]] bool isReady() const;
    [[nodiscard]] const RenderTexture2D &texture() const;
    [[nodiscard]] const MediaProperties &mediaProps() const;

    void loadMedia(const char *file);
    void play();
    void pause();
    void seek(double seconds, bool isAbsolute = false);
    void volume(double value, bool isAbsolute = false);
    void mute();
    void unmute();

    [[nodiscard]] bool isPaused();
    [[nodiscard]] bool isMuted();
    [[nodiscard]] bool hasMedia();
    [[nodiscard]] double duration();
    [[nodiscard]] double volume();
    [[nodiscard]] double currentTime();

    void update();

  private:
    mpv_handle *m_mpvHandle{};
    mpv_render_context *m_mpvRenderCtx{};

    RenderTexture2D m_targetTexture{};

    MediaProperties m_mediaProps{};

    GLFWwindow *m_sharedGLContext{};
    std::thread m_initThread;
    std::atomic<bool> m_isReady{false};

    double m_currentTime{};

    void initWorker();

    bool handleEventLogMessage(mpv_event *event);
    bool handleEventFileLoaded(mpv_event *event);
    bool handleEventVideoReconfig(mpv_event *event);
    bool handleEventPropertyChange(mpv_event *event);
};
}