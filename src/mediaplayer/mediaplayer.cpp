#include "mediaplayer.hpp"
#include "context.hpp"
#include "log.hpp"

#include "GLFW/glfw3.h"
#include "rlgl.h"

#include <cstring>
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
    if (m_initThread.joinable()) { m_initThread.join(); }

    if (m_sharedGLContext)
    {
        glfwDestroyWindow(m_sharedGLContext);
        m_sharedGLContext = nullptr;
    }

    mpv_render_context_free(m_mpvRenderCtx);
    mpv_terminate_destroy(m_mpvHandle);
    UnloadRenderTexture(m_targetTexture);
}

void MediaPlayer::init()
{
    if (context::shouldExit()) { return; }

    GLFWwindow *mainWindow = glfwGetCurrentContext();
    if (!mainWindow) { return context::requestExit("(MediaPlayer::init) Failed to get current window context"); }

    m_sharedGLContext = glfwCreateWindow(1, 1, "", nullptr, mainWindow);
    if (!m_sharedGLContext) { return context::requestExit("(MediaPlayer::init) Failed to create shared GL context for background init"); }

    m_initThread = std::thread(&MediaPlayer::initWorker, this);
}

void MediaPlayer::initWorker()
{
    glfwMakeContextCurrent(m_sharedGLContext);

    m_mpvHandle = mpv_create();
    if (!m_mpvHandle)
    {
        context::requestExit("(MediaPlayer::init) Failed to initialize mpv");
        glfwMakeContextCurrent(nullptr);
        return;
    }

    auto setOpt = [this](const char *name, const char *value) {
        if (auto err = mpv_set_option_string(m_mpvHandle, name, value); err < MPV_ERROR_SUCCESS)
        {
            context::requestExit(std::format("(mpv_set_option_string/{}) {}", name, mpv_error_string(err)).c_str());
            return false;
        }
        return true;
    };

    if (!setOpt("vo", "libmpv") || !setOpt("hwdec", "auto") || !setOpt("keep-open", "yes") || !setOpt("vid", "auto"))
    {
        glfwMakeContextCurrent(nullptr);
        return;
    }

    if (auto err = mpv_initialize(m_mpvHandle); err < MPV_ERROR_SUCCESS)
    {
        context::requestExit(std::format("(mpv_initialize) {}", mpv_error_string(err)).c_str());
        glfwMakeContextCurrent(nullptr);
        return;
    }

    if (auto err = mpv_request_log_messages(m_mpvHandle, "warn"); err < MPV_ERROR_SUCCESS)
    {
        context::requestExit(std::format("(mpv_request_log_messages) {}", mpv_error_string(err)).c_str());
        glfwMakeContextCurrent(nullptr);
        return;
    }

    mpv_opengl_init_params glInitParams{.get_proc_address = getProcAddress};
    mpv_render_param initParams[] = {
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &glInitParams},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };
    if (auto err = mpv_render_context_create(&m_mpvRenderCtx, m_mpvHandle, initParams); err < MPV_ERROR_SUCCESS)
    {
        context::requestExit(std::format("(mpv_render_context_create) {}", mpv_error_string(err)).c_str());
        glfwMakeContextCurrent(nullptr);
        return;
    }

    for (size_t i = 0; i < MediaProperties::properties.size(); i++)
    {
        if (auto err = mpv_observe_property(m_mpvHandle, i + 1, MediaProperties::properties[i].name, MediaProperties::properties[i].format);
            err < MPV_ERROR_SUCCESS)
        {
            context::requestExit(
                std::format("(mpv_observe_property/{}) {}", MediaProperties::properties[i].name, mpv_error_string(err)).c_str());
            glfwMakeContextCurrent(nullptr);
            return;
        }
    }

    mpv_render_context_set_update_callback(m_mpvRenderCtx, onMPVRender, nullptr);
    mpv_set_wakeup_callback(m_mpvHandle, onMPVEvents, nullptr);

    glFinish();
    glfwMakeContextCurrent(nullptr);

    m_isReady.store(true, std::memory_order_release);
}

bool MediaPlayer::isReady() const { return m_isReady.load(std::memory_order_acquire); }
const RenderTexture2D &MediaPlayer::texture() const { return m_targetTexture; }

void MediaPlayer::loadMedia(const char *file)
{
    if (context::shouldExit() || !isReady()) { return; }
    const char *cmd[] = {"loadfile", file, nullptr};
    if (auto err = mpv_command_async(m_mpvHandle, 0, cmd); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_command_async/loadfile) {}", mpv_error_string(err)).c_str());
    }
}

void MediaPlayer::play()
{
    if (context::shouldExit() || !isReady()) { return; }
    int flag = 0;
    if (auto err = mpv_set_property(m_mpvHandle, "pause", MPV_FORMAT_FLAG, &flag); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_set_property/pause) {}", mpv_error_string(err)).c_str());
    }
}

void MediaPlayer::pause()
{
    if (context::shouldExit() || !isReady()) { return; }
    int flag = 1;
    if (auto err = mpv_set_property(m_mpvHandle, "pause", MPV_FORMAT_FLAG, &flag); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_set_property/pause) {}", mpv_error_string(err)).c_str());
    }
}

void MediaPlayer::seek(double seconds, bool isAbsolute)
{
    if (context::shouldExit() || !isReady()) { return; }
    std::string time  = std::format("{}", seconds);
    const char *cmd[] = {"seek", time.c_str(), isAbsolute ? "absolute" : "relative", nullptr};
    if (auto err = mpv_command_async(m_mpvHandle, 0, cmd); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_command_async/seek) {}", mpv_error_string(err)).c_str());
    }
}

void MediaPlayer::volume(double value, bool isAbsolute)
{
    if (context::shouldExit() || !isReady()) { return; }
    double volValue = m_props.get<MediaProperties::PropertyId::Volume>();
    volValue        = std::clamp(isAbsolute ? value : volValue + value, 0.0, 100.0);
    if (auto err = mpv_set_property(m_mpvHandle, "volume", MPV_FORMAT_DOUBLE, &volValue); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_set_property/volume) {}", mpv_error_string(err)).c_str());
    }
}

void MediaPlayer::mute()
{
    if (context::shouldExit() || !isReady()) { return; }
    if (auto err = mpv_set_property_string(m_mpvHandle, "mute", "yes"); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_set_property_string/mute) {}", mpv_error_string(err)).c_str());
    }
}

void MediaPlayer::unmute()
{
    if (context::shouldExit() || !isReady()) { return; }
    if (auto err = mpv_set_property_string(m_mpvHandle, "mute", "no"); err < MPV_ERROR_SUCCESS)
    {
        return context::requestExit(std::format("(mpv_set_property_string/mute) {}", mpv_error_string(err)).c_str());
    }
}

// ---------- PROPERTIES ----------

double MediaPlayer::duration() { return m_props.get<MediaProperties::PropertyId::Duration>(); }
double MediaPlayer::currentTime() { return m_props.get<MediaProperties::PropertyId::CurrentTime>(); }
double MediaPlayer::cacheTime() { return m_props.get<MediaProperties::PropertyId::CacheTime>(); }
double MediaPlayer::volume() { return m_props.get<MediaProperties::PropertyId::Volume>(); }

int64_t MediaPlayer::videoWidth() { return m_props.get<MediaProperties::PropertyId::VideoWidth>(); }
int64_t MediaPlayer::videoHeight() { return m_props.get<MediaProperties::PropertyId::VideoHeight>(); }
int64_t MediaPlayer::frameDropCount() { return m_props.get<MediaProperties::PropertyId::FrameDropCount>(); }

const std::string &MediaPlayer::title() { return m_props.get<MediaProperties::PropertyId::Title>(); }
const std::string &MediaPlayer::filename() { return m_props.get<MediaProperties::PropertyId::Filename>(); }
const std::string &MediaPlayer::videoCodec() { return m_props.get<MediaProperties::PropertyId::VideoCodec>(); }
const std::string &MediaPlayer::hwdec() { return m_props.get<MediaProperties::PropertyId::Hwdec>(); }
const std::string &MediaPlayer::vo() { return m_props.get<MediaProperties::PropertyId::VO>(); }
const std::string &MediaPlayer::ao() { return m_props.get<MediaProperties::PropertyId::AO>(); }

bool MediaPlayer::isMuted() { return m_props.get<MediaProperties::PropertyId::IsMuted>(); }
bool MediaPlayer::isPaused() { return m_props.get<MediaProperties::PropertyId::IsPaused>(); }
bool MediaPlayer::isPausedForCache() { return m_props.get<MediaProperties::PropertyId::IsPausedForCache>(); }
bool MediaPlayer::hasReachedEnd() { return m_props.get<MediaProperties::PropertyId::HasReachedEnd>(); }
bool MediaPlayer::hasMedia() { return m_props.get<MediaProperties::PropertyId::Duration>() > 0.0; }

void MediaPlayer::update()
{
    if (context::shouldExit()) { return; }

    if (m_initThread.joinable() && m_isReady.load(std::memory_order_acquire))
    {
        m_initThread.join();
        glfwDestroyWindow(m_sharedGLContext);
        m_sharedGLContext = nullptr;
    }

    if (!isReady()) { return; }

    if (g_mpvEvents.exchange(false))
    {
        mpv_event *event = mpv_wait_event(m_mpvHandle, 0);
        while (event->event_id != MPV_EVENT_NONE)
        {
            switch (event->event_id)
            {
            case MPV_EVENT_LOG_MESSAGE:
                if (!handleEventLogMessage(event)) { return; }
                break;
            case MPV_EVENT_VIDEO_RECONFIG:
                if (!handleEventVideoReconfig(event)) { return; }
                break;
            case MPV_EVENT_PROPERTY_CHANGE:
                if (!handleEventPropertyChange(event)) { return; }
                break;
            default: break;
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

bool MediaPlayer::handleEventLogMessage(mpv_event *event)
{
    if (context::shouldExit()) { return false; }
    auto *msg = static_cast<mpv_event_log_message *>(event->data);
    if (!msg) { return false; }
    logger::warning("({}) {}", msg->prefix, msg->text);
    return true;
}

bool MediaPlayer::handleEventVideoReconfig(mpv_event *event)
{
    if (context::shouldExit()) { return false; }
    int64_t width{};
    int64_t height{};
    if (auto err = mpv_get_property(m_mpvHandle, "dwidth", MPV_FORMAT_INT64, &width); err < MPV_ERROR_SUCCESS)
    {
        logger::warning("(mpv_get_property/dwidth) {}", mpv_error_string(err));
    }

    if (auto err = mpv_get_property(m_mpvHandle, "dheight", MPV_FORMAT_INT64, &height); err < MPV_ERROR_SUCCESS)
    {
        logger::warning("(mpv_get_property/dheight) {}", mpv_error_string(err));
    }

    if (width > 0 && height > 0)
    {
        UnloadRenderTexture(m_targetTexture);
        m_targetTexture = LoadRenderTexture(width, height);
        SetTextureFilter(m_targetTexture.texture, TEXTURE_FILTER_BILINEAR);
    }
    else
    {
        logger::warning("(mpv_get_property/dwidth&dheight) {}", "Video dimensions still unavailable");
    }
    return true;
}

bool MediaPlayer::handleEventPropertyChange(mpv_event *event)
{
    if (context::shouldExit()) { return false; }
    auto prop = static_cast<mpv_event_property *>(event->data);
    if (!prop) { return false; }

    m_props.update(event->reply_userdata, *prop);
    return true;
}
}