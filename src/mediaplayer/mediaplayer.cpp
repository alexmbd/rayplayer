#include "mediaplayer.hpp"
#include "context.hpp"
#include "log.hpp"

#include "rlgl.h"

#include <atomic>
#include <format>

namespace Rayplayer
{
namespace
{
void *getProcAddress(void *ctx, const char *name)
{
    auto proc = rlGetProcAddress(name);
    if (!proc && !context::shouldExit()) { context::requestExit("(getProcAddress) OpenGL procedure address is nullptr"); }
    return proc;
}

std::atomic<bool> g_mpvRender{true};
std::atomic<bool> g_mpvEvents{false};
void onMPVRender(void *ctx) { g_mpvRender.store(true, std::memory_order_relaxed); }
void onMPVEvents(void *data) { g_mpvEvents.store(true, std::memory_order_relaxed); }
}

MediaPlayer::~MediaPlayer()
{
    mpv_render_context_free(m_mpvRenderCtx);
    mpv_terminate_destroy(m_mpvHandle);
    UnloadRenderTexture(m_targetTexture);
}

void MediaPlayer::init()
{
    if (context::shouldExit()) { return; }

    m_mpvHandle = mpv_create();
    if (!m_mpvHandle) { return context::requestExit("(MediaPlayer::init) Failed to initialize mpv"); }

    if (auto err = mpv_set_option_string(m_mpvHandle, "vo", "libmpv"); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_set_option_string/vo) {}", mpv_error_string(err)).c_str());
    }

    if (auto err = mpv_set_option_string(m_mpvHandle, "hwdec", "auto"); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_set_option_string/hwdec) {}", mpv_error_string(err)).c_str());
    }

    if (auto err = mpv_set_option_string(m_mpvHandle, "keep-open", "yes"); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_set_option_string/keep-open) {}", mpv_error_string(err)).c_str());
    }

    if (auto err = mpv_set_option_string(m_mpvHandle, "vid", "auto"); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_set_option_string/vid) {}", mpv_error_string(err)).c_str());
    }

    if (auto err = mpv_initialize(m_mpvHandle); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_initialize) {}", mpv_error_string(err)).c_str());
    }

    if (auto err = mpv_request_log_messages(m_mpvHandle, "warn"); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_request_log_messages) {}", mpv_error_string(err)).c_str());
    }

    mpv_opengl_init_params glInitParams{.get_proc_address = getProcAddress};
    mpv_render_param initParams[] = {
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &glInitParams},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };
    if (auto err = mpv_render_context_create(&m_mpvRenderCtx, m_mpvHandle, initParams); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_render_context_create) {}", mpv_error_string(err)).c_str());
    }

    mpv_render_context_set_update_callback(m_mpvRenderCtx, onMPVRender, nullptr);
    mpv_set_wakeup_callback(m_mpvHandle, onMPVEvents, nullptr);
}

const RenderTexture2D &MediaPlayer::texture() const { return m_targetTexture; }

const MediaProperties &MediaPlayer::mediaProps() const { return m_mediaProps; }

bool MediaPlayer::isPaused()
{
    if (context::shouldExit()) { return false; }
    int paused = 0;
    if (auto err = mpv_get_property(m_mpvHandle, "pause", MPV_FORMAT_FLAG, &paused); err < MPV_ERROR_SUCCESS)
    {
        context::requestExit(std::format("(mpv_get_property/pause) {}", mpv_error_string(err)).c_str());
    }
    return paused == 1;
}

void MediaPlayer::loadMedia(const char *file)
{
    if (context::shouldExit()) { return; }
    const char *cmd[] = {"loadfile", file, nullptr};
    if (auto err = mpv_command_async(m_mpvHandle, 0, cmd); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_command_async/loadfile) {}", mpv_error_string(err)).c_str());
    }
}

void MediaPlayer::play()
{
    if (context::shouldExit()) { return; }
    int flag = 0;
    if (auto err = mpv_set_property(m_mpvHandle, "pause", MPV_FORMAT_FLAG, &flag); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_set_property/pause) {}", mpv_error_string(err)).c_str());
    }
}

void MediaPlayer::pause()
{
    if (context::shouldExit()) { return; }
    int flag = 1;
    if (auto err = mpv_set_property(m_mpvHandle, "pause", MPV_FORMAT_FLAG, &flag); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_set_property/pause) {}", mpv_error_string(err)).c_str());
    }
}

void MediaPlayer::seek(double secondsDelta)
{
    if (context::shouldExit()) { return; }
    const char *cmd[] = {"seek", std::format("{}", secondsDelta).c_str(), "relative", nullptr};
    if (auto err = mpv_command_async(m_mpvHandle, 0, cmd); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_command_async/seek) {}", mpv_error_string(err)).c_str());
    }
}

void MediaPlayer::volume(double valueDelta)
{
    if (context::shouldExit()) { return; }
    double value = 0.0;
    if (auto err = mpv_get_property(m_mpvHandle, "volume", MPV_FORMAT_DOUBLE, &value); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_get_property/volume) {}", mpv_error_string(err)).c_str());
    }

    value = std::clamp(value + valueDelta, 0.0, 100.0);
    if (auto err = mpv_set_property(m_mpvHandle, "volume", MPV_FORMAT_DOUBLE, &value); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_set_property/volume) {}", mpv_error_string(err)).c_str());
    }
}

void MediaPlayer::update()
{
    if (context::shouldExit()) { return; }

    if (g_mpvEvents.exchange(false))
    {
        mpv_event *event = mpv_wait_event(m_mpvHandle, 0);
        while (event->event_id != MPV_EVENT_NONE)
        {
            if (event->event_id == MPV_EVENT_LOG_MESSAGE)
            {
                auto *msg = static_cast<mpv_event_log_message *>(event->data);
                logger::warning("({}) {}", msg->prefix, msg->text);
            }
            else if (event->event_id == MPV_EVENT_FILE_LOADED)
            {
                if (context::shouldExit()) { return; }
                char *codec = mpv_get_property_string(m_mpvHandle, "video-codec");
                if (codec) { m_mediaProps.videoCodec = codec; }
                else
                {
                    context::requestExit("(mpv_get_property_string/video-codec) Failed to get video codec");
                }
                mpv_free(codec);
            }
            else if (event->event_id == MPV_EVENT_VIDEO_RECONFIG)
            {
                if (context::shouldExit()) { return; }
                if (auto err = mpv_get_property(m_mpvHandle, "dwidth", MPV_FORMAT_INT64, &m_mediaProps.videoWidth); err < MPV_ERROR_SUCCESS)
                {
                    return context::requestExit(std::format("(mpv_get_property/dwidth) {}", mpv_error_string(err)).c_str());
                }

                if (auto err = mpv_get_property(m_mpvHandle, "dheight", MPV_FORMAT_INT64, &m_mediaProps.videoHeight);
                    err < MPV_ERROR_SUCCESS)
                {
                    return context::requestExit(std::format("(mpv_get_property/dheight) {}", mpv_error_string(err)).c_str());
                }

                if (m_mediaProps.videoWidth > 0 && m_mediaProps.videoHeight > 0)
                {
                    UnloadRenderTexture(m_targetTexture);
                    m_targetTexture = LoadRenderTexture(m_mediaProps.videoWidth, m_mediaProps.videoHeight);
                }
                else
                {
                    logger::warning("(mpv_get_property/dwidth&dheight) {}", "Video dimensions still unavailable");
                }
            }

            event = mpv_wait_event(m_mpvHandle, 0);
        }
    }

    if (g_mpvRender.exchange(false))
    {
        if (mpv_render_context_update(m_mpvRenderCtx) & MPV_RENDER_UPDATE_FRAME)
        {
            if (context::shouldExit()) { return; }
            if (m_targetTexture.id == 0 || m_targetTexture.texture.width <= 0 || m_targetTexture.texture.height <= 0) { return; }

            rlDisableBackfaceCulling();
            rlViewport(0, 0, m_targetTexture.texture.width, m_targetTexture.texture.height);

            mpv_opengl_fbo fbo{
                .fbo             = static_cast<int>(m_targetTexture.id),
                .w               = m_targetTexture.texture.width,
                .h               = m_targetTexture.texture.height,
                .internal_format = 0,
            };

            int flipY                       = 1;
            mpv_render_param renderParams[] = {
                {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
                {MPV_RENDER_PARAM_FLIP_Y, &flipY},
                {MPV_RENDER_PARAM_INVALID, nullptr},
            };

            if (auto err = mpv_render_context_render(m_mpvRenderCtx, renderParams); err < MPV_ERROR_SUCCESS)
            {
                context::requestExit(std::format("(mpv_render_context_render) {}", mpv_error_string(err)).c_str());
            }
            rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());
            rlEnableBackfaceCulling();
        }
    }
}
}