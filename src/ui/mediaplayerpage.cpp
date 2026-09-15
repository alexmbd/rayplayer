#include "mediaplayerpage.hpp"

#include "rlgl.h"

#include <vector>

namespace Rayplayer
{
namespace
{
namespace Colors
{
constexpr Color GradientTop{0, 0, 0, 170};
constexpr Color GradientBottom{0, 0, 0, 190};
constexpr Color Transparent{0, 0, 0, 0};

constexpr Color ProgressPlayed{255, 0, 0, 255};
constexpr Color ProgressBuffered{255, 255, 255, 100};
constexpr Color ProgressBase{255, 255, 255, 60};
constexpr Color ScrubberHandle{255, 0, 0, 255};

constexpr Color TextPrimary{255, 255, 255, 255};
constexpr Color TextSecondary{255, 255, 255, 180};

constexpr Color HoverOverlay{255, 255, 255, 40};
constexpr Color IconTint{255, 255, 255, 255};
}

namespace PageConstants
{
constexpr float TopBarHeight{64.0f};
constexpr float BottomBarHeight{64.0f};
constexpr float ProgressHeight{4.0f};
constexpr float ProgressHitBoxPadding{8.0f};
constexpr float Margin{16.0f};
constexpr float ControlSpacing{16.0f};
constexpr float VolumeSliderWidth{60.0f};

constexpr float IconSize{32.0f};
constexpr float IconButtonPadding{8.0f};

constexpr float IdleTimeoutSeconds{3.0f};
constexpr float FadeSpeed{8.0f};
constexpr float MouseMoveThreshold{1.0f};
}

Rectangle letterboxedDst(int videoW, int videoH, int winW, int winH)
{
    float videoAspect = static_cast<float>(videoW) / videoH;
    float winAspect   = static_cast<float>(winW) / winH;

    float dstW        = 0.0f;
    float dstH        = 0.0f;
    if (winAspect > videoAspect)
    {
        dstH = static_cast<float>(winH);
        dstW = dstH * videoAspect;
    }
    else
    {
        dstW = static_cast<float>(winW);
        dstH = dstW / videoAspect;
    }

    float x = (winW - dstW) * 0.5f;
    float y = (winH - dstH) * 0.5f;
    return Rectangle{x, y, dstW, dstH};
}
}

MediaPlayerPage::MediaPlayerPage(Font *font)
{
    m_font = font;
    m_mediaPlayer.init();
    SetTextureFilter(m_mediaPlayer.texture().texture, TEXTURE_FILTER_BILINEAR);

    m_textures["pause"]      = LoadTexture("resources/icons/pause32.png");
    m_textures["play"]       = LoadTexture("resources/icons/play32.png");
    m_textures["volume"]     = LoadTexture("resources/icons/volume32.png");
    m_textures["volumehigh"] = LoadTexture("resources/icons/volumehigh32.png");
    m_textures["volumelow"]  = LoadTexture("resources/icons/volumelow32.png");
    m_textures["volumemute"] = LoadTexture("resources/icons/volumemute32.png");
    for (const auto &[_, texture] : m_textures) { SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR); }

    m_progressSlider.trackColor(Colors::ProgressBase);
    m_progressSlider.fillColor(Colors::ProgressPlayed);
    m_progressSlider.handleColor(Colors::ScrubberHandle);
    m_progressSlider.onValueChange = [this](double value) {
        if (!m_mediaPlayer.hasMedia()) { return; }
        m_mediaPlayer.seek(value * m_mediaPlayer.duration(), true);
    };

    m_playBtn.icon(std::addressof(m_textures["play"]));
    m_playBtn.iconSize(PageConstants::IconSize);
    m_playBtn.onClick = [this]() {
        if (!m_mediaPlayer.hasMedia()) { return; }
        if (m_mediaPlayer.isPaused())
        {
            m_playBtn.icon(std::addressof(m_textures["pause"]));
            m_mediaPlayer.play();
        }
        else
        {
            m_playBtn.icon(std::addressof(m_textures["play"]));
            m_mediaPlayer.pause();
        }
    };

    m_volumeBtn.icon(std::addressof(m_textures["volumehigh"]));
    m_volumeBtn.iconSize(PageConstants::IconSize);
    m_volumeBtn.onClick = [this]() {
        if (!m_mediaPlayer.hasMedia()) { return; }
        if (m_mediaPlayer.isMuted())
        {
            m_volumeBtn.icon(std::addressof(m_textures["volumehigh"]));
            m_mediaPlayer.unmute();
        }
        else
        {
            m_volumeBtn.icon(std::addressof(m_textures["volumemute"]));
            m_mediaPlayer.mute();
        }
    };

    m_volumeSlider.trackColor(Colors::ProgressBase);
    m_volumeSlider.onValueChange = [this](double value) {
        if (!m_mediaPlayer.hasMedia()) { return; }
        m_mediaPlayer.volume(value * 100.0, true);
    };
}

MediaPlayerPage::~MediaPlayerPage()
{
    for (const auto &[_, texture] : m_textures) { UnloadTexture(texture); }
}

void MediaPlayerPage::update()
{
    if (IsFileDropped())
    {
        FilePathList droppedFiles = LoadDroppedFiles();
        std::vector<std::string> droppedFilePaths;
        for (uint32_t i = 0; i < droppedFiles.count; i++) { droppedFilePaths.push_back(droppedFiles.paths[i]); }
        UnloadDroppedFiles(droppedFiles);
        if (droppedFilePaths.size() >= 1) { m_mediaPlayer.loadMedia(droppedFilePaths[0].c_str()); }
        m_playBtn.icon(std::addressof(m_textures["pause"]));
    }

    if (m_mediaPlayer.hasMedia())
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            if (m_mediaPlayer.isPaused())
            {
                m_playBtn.icon(std::addressof(m_textures["pause"]));
                m_mediaPlayer.play();
            }
            else
            {
                m_playBtn.icon(std::addressof(m_textures["play"]));
                m_mediaPlayer.pause();
            }
        }
        else if (IsKeyPressed(KEY_LEFT)) { m_mediaPlayer.seek(-5.0); }
        else if (IsKeyPressed(KEY_RIGHT)) { m_mediaPlayer.seek(5.0); }
        else if (IsKeyPressed(KEY_UP)) { m_mediaPlayer.volume(5.0); }
        else if (IsKeyPressed(KEY_DOWN)) { m_mediaPlayer.volume(-5.0); }
        else if (IsKeyPressed(KEY_F)) { ToggleFullscreen(); }
    }

    m_mediaPlayer.update();
    computeLayout();

    auto mouseEvent = MouseEvent{MOUSE_BUTTON_LEFT};
    m_progressSlider.value(m_mediaPlayer.currentTime() / (m_mediaPlayer.duration() == 0.0 ? 1.0 : m_mediaPlayer.duration()));
    m_progressSlider.update(mouseEvent);
    m_playBtn.update(mouseEvent);
    m_volumeBtn.update(mouseEvent);
    m_volumeSlider.value(m_mediaPlayer.volume() / 100.0);
    m_volumeSlider.update(mouseEvent);
}

void MediaPlayerPage::draw()
{
    ClearBackground(BLACK);

    if (m_mediaPlayer.isReady())
    {
        const RenderTexture2D &target           = m_mediaPlayer.texture();
        const Rayplayer::MediaProperties &props = m_mediaPlayer.mediaProps();
        Rectangle src{0, 0, static_cast<float>(target.texture.width), -static_cast<float>(target.texture.height)};
        Rectangle dst = letterboxedDst(props.videoWidth, props.videoHeight, GetScreenWidth(), GetScreenHeight());

        rlDisableColorBlend();
        DrawTexturePro(target.texture, src, dst, {0.0f, 0.0f}, 0.0f, WHITE);
        rlEnableColorBlend();
    }

    m_progressSlider.draw();
    m_playBtn.draw();
    m_volumeBtn.draw();
    m_volumeSlider.draw();
}

void MediaPlayerPage::computeLayout()
{
    const float screenW    = static_cast<float>(GetScreenWidth());
    const float screenH    = static_cast<float>(GetScreenHeight());

    const float bottomBarY = screenH - PageConstants::BottomBarHeight;
    const float controlsY  = bottomBarY + PageConstants::ProgressHeight + 8.0f;

    m_progressSlider.bounds(Rectangle{PageConstants::Margin, bottomBarY - PageConstants::ProgressHeight * 0.5f,
                                      screenW - PageConstants::Margin * 2.0f, PageConstants::ProgressHeight});

    float x = PageConstants::Margin;
    m_playBtn.bounds(Rectangle{x, controlsY, m_playBtn.iconSize() + PageConstants::IconButtonPadding,
                               m_playBtn.iconSize() + PageConstants::IconButtonPadding});
    x += m_playBtn.iconSize() + PageConstants::IconButtonPadding + PageConstants::ControlSpacing;

    m_volumeBtn.bounds(Rectangle{x, controlsY, m_volumeBtn.iconSize() + PageConstants::IconButtonPadding,
                                 m_volumeBtn.iconSize() + PageConstants::IconButtonPadding});
    x += m_volumeBtn.iconSize() + PageConstants::IconButtonPadding + PageConstants::ControlSpacing - 4.0f;

    m_volumeSlider.bounds(Rectangle{x, controlsY + (m_volumeBtn.iconSize() + PageConstants::IconButtonPadding) * 0.45f,
                                    PageConstants::VolumeSliderWidth, PageConstants::ProgressHeight});
}
}