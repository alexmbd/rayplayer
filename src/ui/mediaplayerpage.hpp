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
    struct Colors
    {
        static constexpr Color GradientTop{0, 0, 0, 170};
        static constexpr Color GradientBottom{0, 0, 0, 190};
        static constexpr Color Transparent{0, 0, 0, 0};

        static constexpr Color ProgressPlayed{255, 0, 0, 255};
        static constexpr Color ProgressBuffered{255, 255, 255, 100};
        static constexpr Color ProgressBase{255, 255, 255, 60};
        static constexpr Color ScrubberHandle{255, 0, 0, 255};

        static constexpr Color TextPrimary{255, 255, 255, 255};
        static constexpr Color TextSecondary{255, 255, 255, 180};

        static constexpr Color HoverOverlay{255, 255, 255, 40};
        static constexpr Color IconTint{255, 255, 255, 255};
    };

    struct PageConstants
    {
        static constexpr float TopBarHeight{64.0f};
        static constexpr float BottomBarHeight{64.0f};
        static constexpr float ProgressHeight{4.0f};
        static constexpr float ProgressHitBoxPadding{8.0f};
        static constexpr float Margin{16.0f};
        static constexpr float ControlSpacing{14.0f};
        static constexpr float VolumeSliderWidth{60.0f};

        static constexpr float IconSize{24.0f};
        static constexpr float IconButtonPadding{10.0f};

        static constexpr float IdleTimeoutSeconds{3.0f};
        static constexpr float FadeSpeed{8.0f};
        static constexpr float MouseMoveThreshold{1.0f};
    };

    struct MediaInfo
    {
        double duration{0.0};
        double currentTime{0.0};
    };

    struct InteractionStates
    {
        bool isScrubbing{false};
        bool isDraggingVolume{false};
        bool isHoveringProgress{false};
        bool isHoveringPlay{false};
        bool isHoveringVolumeIcon{false};
        bool isHoveringVolumeSlider{false};
        bool isHoveringFullscreen{false};
        bool isHoveringSettings{false};
    };

    struct ElementRects
    {
        Rectangle topBar{};
        Rectangle bottomBar{};
        Rectangle progress{};
        Rectangle progressHitBox{};
        Rectangle playButton{};
        Rectangle volumeIcon{};
        Rectangle volumeSlider{};
        Rectangle settings{};
        Rectangle fullscreen{};
    };

    MediaPlayer m_mediaPlayer;

    MediaInfo m_mediaInfo;
    InteractionStates m_interactionStates;
    ElementRects m_elementRects;

    void computeLayout();
};
}