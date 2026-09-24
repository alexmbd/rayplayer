#pragma once

#include "mpv/client.h"
#include "mpv/render_gl.h"
#include "raylib.h"

#include <array>
#include <atomic>
#include <string>
#include <thread>

struct GLFWwindow;

namespace Rayplayer
{
namespace MediaProperties
{
enum class PropertyId : uint64_t
{
    // Double
    Duration,
    CurrentTime,
    CacheTime,
    Volume,

    // Int64
    VideoWidth,
    VideoHeight,
    FrameDropCount,

    // String
    Title,
    Filename,
    VideoCodec,
    Hwdec,
    VO,
    AO,

    // Bool
    IsMuted,
    IsPaused,
    IsPausedForCache,
    HasReachedEnd,

    Count
};

struct Property
{
    const char *name;
    mpv_format format;
};

// The order of properties here must be the same as the order in the PropertyId enum
inline constexpr std::array<Property, static_cast<size_t>(PropertyId::Count)> properties{
    {{"duration", MPV_FORMAT_DOUBLE}, // All time based properties are in seconds
     {"time-pos", MPV_FORMAT_DOUBLE},
     {"demuxer-cache-time", MPV_FORMAT_DOUBLE},
     {"volume", MPV_FORMAT_DOUBLE},
     {"dwidth", MPV_FORMAT_INT64},
     {"dheight", MPV_FORMAT_INT64},
     {"frame-drop-count", MPV_FORMAT_INT64},
     {"media-title", MPV_FORMAT_STRING},
     {"filename", MPV_FORMAT_STRING},
     {"video-codec", MPV_FORMAT_STRING},
     {"hwdec-current", MPV_FORMAT_STRING},
     {"current-vo", MPV_FORMAT_STRING},
     {"current-ao", MPV_FORMAT_STRING},
     {"mute", MPV_FORMAT_FLAG},
     {"pause", MPV_FORMAT_FLAG},
     {"paused-for-cache", MPV_FORMAT_FLAG},
     {"eof-reached", MPV_FORMAT_FLAG}}};

inline constexpr auto indexMapping = []() {
    std::array<uint64_t, properties.size()> map{};
    uint64_t doubleCount{};
    uint64_t int64Count{};
    uint64_t stringCount{};
    uint64_t boolCount{};
    for (size_t i = 0; i < map.size(); i++)
    {
        switch (properties[i].format)
        {
        case MPV_FORMAT_DOUBLE: map[i] = doubleCount++; break;
        case MPV_FORMAT_INT64: map[i] = int64Count++; break;
        case MPV_FORMAT_STRING: map[i] = stringCount++; break;
        case MPV_FORMAT_FLAG: map[i] = boolCount++; break;
        default: break;
        }
    }
    return map;
}();

inline constexpr size_t countPropFormat(mpv_format format)
{
    size_t total{};
    for (const auto &prop : properties)
    {
        if (prop.format == format) { total++; }
    }
    return total;
}

template <mpv_format F> struct FormatType;
template <> struct FormatType<MPV_FORMAT_DOUBLE>
{
    using type = double;
};
template <> struct FormatType<MPV_FORMAT_INT64>
{
    using type = int64_t;
};
template <> struct FormatType<MPV_FORMAT_STRING>
{
    using type = std::string;
};
template <> struct FormatType<MPV_FORMAT_FLAG>
{
    using type = bool;
};

class ObservedProperties final
{
  public:
    template <PropertyId P> using valueType                 = typename FormatType<properties[static_cast<size_t>(P)].format>::type;

    ObservedProperties()                                    = default;
    ~ObservedProperties()                                   = default;

    ObservedProperties(ObservedProperties &&other) noexcept = delete;
    ObservedProperties &operator=(ObservedProperties &&other) noexcept = delete;

    ObservedProperties(const ObservedProperties &)                     = delete;
    ObservedProperties &operator=(const ObservedProperties &)          = delete;

    template <PropertyId P> [[nodiscard]] decltype(auto) get() const
    {
        using T                = valueType<P>;
        constexpr size_t index = indexMapping[static_cast<size_t>(P)];

        if constexpr (std::same_as<T, double>) { return m_propDouble[index]; }
        else if constexpr (std::same_as<T, int64_t>) { return m_propInt64[index]; }
        else if constexpr (std::same_as<T, bool>) { return m_propBool[index]; }
        else
        {
            return static_cast<const std::string &>(m_propString[index]);
        }
    }

    template <PropertyId P> void set(valueType<P> value)
    {
        using T                = valueType<P>;
        constexpr size_t index = indexMapping[static_cast<size_t>(P)];

        if constexpr (std::same_as<T, double>) { m_propDouble[index] = value; }
        else if constexpr (std::same_as<T, int64_t>) { m_propInt64[index] = value; }
        else if constexpr (std::same_as<T, bool>) { m_propBool[index] = value; }
        else
        {
            m_propString[index] = std::move(value);
        }
    }

    void update(uint64_t userdata, const mpv_event_property &prop)
    {
        if ((userdata == 0) || (userdata > properties.size())) { return; }

        const size_t id      = userdata - 1;
        const size_t index   = indexMapping[id];
        const mpv_format fmt = properties[id].format;
        const bool hasData   = (prop.data != nullptr) && (prop.format == fmt);

        switch (fmt)
        {
        case MPV_FORMAT_DOUBLE: m_propDouble[index] = hasData ? *static_cast<double *>(prop.data) : 0.0; break;
        case MPV_FORMAT_INT64: m_propInt64[index] = hasData ? *static_cast<int64_t *>(prop.data) : 0; break;
        case MPV_FORMAT_FLAG: m_propBool[index] = hasData && *static_cast<int32_t *>(prop.data) != 0; break;
        case MPV_FORMAT_STRING: {
            const char *str = hasData ? *static_cast<char **>(prop.data) : nullptr;
            m_propString[index].assign(str ? str : "");
            break;
        }
        default: break;
        }
    }

    void reset()
    {
        for (auto &prop : m_propDouble) { prop = 0.0; }
        for (auto &prop : m_propInt64) { prop = 0; }
        for (auto &prop : m_propBool) { prop = false; }
        for (auto &prop : m_propString) { prop.assign(""); }
    }

  private:
    std::array<double, countPropFormat(MPV_FORMAT_DOUBLE)> m_propDouble{};
    std::array<int64_t, countPropFormat(MPV_FORMAT_INT64)> m_propInt64{};
    std::array<std::string, countPropFormat(MPV_FORMAT_STRING)> m_propString{};
    std::array<bool, countPropFormat(MPV_FORMAT_FLAG)> m_propBool{};
};
}

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

    void loadMedia(const char *file);
    void play();
    void pause();
    void seek(double seconds, bool isAbsolute = false);
    void volume(double value, bool isAbsolute = false);
    void mute();
    void unmute();

    // ---------- PROPERTIES ----------

    [[nodiscard]] double duration();
    [[nodiscard]] double currentTime();
    [[nodiscard]] double cacheTime();
    [[nodiscard]] double volume();

    [[nodiscard]] int64_t videoWidth();
    [[nodiscard]] int64_t videoHeight();
    [[nodiscard]] int64_t frameDropCount();

    [[nodiscard]] const std::string &title();
    [[nodiscard]] const std::string &filename();
    [[nodiscard]] const std::string &videoCodec();
    [[nodiscard]] const std::string &hwdec();
    [[nodiscard]] const std::string &vo();
    [[nodiscard]] const std::string &ao();

    [[nodiscard]] bool isMuted();
    [[nodiscard]] bool isPaused();
    [[nodiscard]] bool isPausedForCache();
    [[nodiscard]] bool hasReachedEnd();
    [[nodiscard]] bool hasMedia();

    void update();

  private:
    mpv_handle *m_mpvHandle{};
    mpv_render_context *m_mpvRenderCtx{};

    RenderTexture2D m_targetTexture{};

    GLFWwindow *m_sharedGLContext{};
    std::thread m_initThread;
    std::atomic<bool> m_isReady{false};

    MediaProperties::ObservedProperties m_props{};

    void initWorker();

    bool handleEventLogMessage(mpv_event *event);
    bool handleEventVideoReconfig(mpv_event *event);
    bool handleEventPropertyChange(mpv_event *event);
};
}