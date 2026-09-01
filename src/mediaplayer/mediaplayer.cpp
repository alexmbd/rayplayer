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
void mpvCall(int errorCode, const char *name)
{
    if (errorCode >= MPV_ERROR_SUCCESS) { return; }
    context::requestExit(std::format("({}) {}", name, mpv_error_string(errorCode)).c_str());
}

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
    if (!m_mpvHandle) { context::requestExit("(MediaPlayer::init) Failed to initialize mpv"); }

    if (context::shouldExit()) { return; }
    mpvCall(mpv_set_option_string(m_mpvHandle, "vo", "libmpv"), "mpv_set_option_string/vo");

    if (context::shouldExit()) { return; }
    mpvCall(mpv_set_option_string(m_mpvHandle, "hwdec", "auto"), "mpv_set_option_string/hwdec");

    if (context::shouldExit()) { return; }
    mpvCall(mpv_set_option_string(m_mpvHandle, "keep-open", "yes"), "mpv_set_option_string/keep-open");

    if (context::shouldExit()) { return; }
    mpvCall(mpv_set_option_string(m_mpvHandle, "vid", "auto"), "mpv_set_option_string/vid");

    if (context::shouldExit()) { return; }
    mpvCall(mpv_initialize(m_mpvHandle), "mpv_initialize");

    if (context::shouldExit()) { return; }
    mpvCall(mpv_request_log_messages(m_mpvHandle, "warn"), "mpv_request_log_messages");

    if (context::shouldExit()) { return; }
    mpv_opengl_init_params glInitParams{.get_proc_address = getProcAddress};
    mpv_render_param initParams[] = {
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &glInitParams},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };
    mpvCall(mpv_render_context_create(&m_mpvRenderCtx, m_mpvHandle, initParams), "mpv_render_context_create");

    mpv_render_context_set_update_callback(m_mpvRenderCtx, onMPVRender, nullptr);
    mpv_set_wakeup_callback(m_mpvHandle, onMPVEvents, nullptr);
}

const RenderTexture2D &MediaPlayer::texture() const { return m_targetTexture; }

const MediaProperties &MediaPlayer::mediaProps() const { return m_mediaProps; }

bool MediaPlayer::isPaused()
{
    if (context::shouldExit()) { return false; }
    int paused = 0;
    mpvCall(mpv_get_property(m_mpvHandle, "pause", MPV_FORMAT_FLAG, &paused), "mpv_get_property/pause");
    return paused == 1;
}

void MediaPlayer::loadMedia(const char *file)
{
    if (context::shouldExit()) { return; }
    const char *cmd[] = {"loadfile", file, nullptr};
    mpvCall(mpv_command_async(m_mpvHandle, 0, cmd), "mpv_command_async/loadfile");
}

void MediaPlayer::play()
{
    if (context::shouldExit()) { return; }
    int flag = 0;
    mpvCall(mpv_set_property(m_mpvHandle, "pause", MPV_FORMAT_FLAG, &flag), "mpv_set_property/pause");
}

void MediaPlayer::pause()
{
    if (context::shouldExit()) { return; }
    int flag = 1;
    mpvCall(mpv_set_property(m_mpvHandle, "pause", MPV_FORMAT_FLAG, &flag), "mpv_set_property/pause");
}

void MediaPlayer::seek(double secondsDelta)
{
    if (context::shouldExit()) { return; }
    const char *cmd[] = {"seek", std::format("{}", secondsDelta).c_str(), "relative", nullptr};
    mpvCall(mpv_command_async(m_mpvHandle, 0, cmd), "mpv_command_async/seek");
}

void MediaPlayer::volume(double valueDelta)
{
    if (context::shouldExit()) { return; }
    double value = 0.0;
    mpvCall(mpv_get_property(m_mpvHandle, "volume", MPV_FORMAT_DOUBLE, &value), "mpv_get_property/volume");

    if (context::shouldExit()) { return; }
    value = std::clamp(value + valueDelta, 0.0, 100.0);
    mpvCall(mpv_set_property(m_mpvHandle, "volume", MPV_FORMAT_DOUBLE, &value), "mpv_set_property/volume");
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
                mpvCall(mpv_get_property(m_mpvHandle, "dwidth", MPV_FORMAT_INT64, &m_mediaProps.videoWidth), "mpv_get_property/dwidth");

                if (context::shouldExit()) { return; }
                mpvCall(mpv_get_property(m_mpvHandle, "dheight", MPV_FORMAT_INT64, &m_mediaProps.videoHeight), "mpv_get_property/dheight");

                if (m_mediaProps.videoWidth > 0 && m_mediaProps.videoHeight > 0)
                {
                    UnloadRenderTexture(m_targetTexture);
                    m_targetTexture = LoadRenderTexture(m_mediaProps.videoWidth, m_mediaProps.videoHeight);
                }
                else
                {
                    logger::warning("(mpv_get_property/dwidth&dheight)", "Video dimensions still unavailable");
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

            mpvCall(mpv_render_context_render(m_mpvRenderCtx, renderParams), "mpv_render_context_render");
            rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());
            rlEnableBackfaceCulling();
        }
    }
}
}