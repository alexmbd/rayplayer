#include "mediaplayerpage.hpp"

#include "rlgl.h"

#include <vector>

namespace Rayplayer
{
namespace
{
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

MediaPlayerPage::MediaPlayerPage() { m_mediaPlayer.init(); }

void MediaPlayerPage::update()
{
    if (IsFileDropped())
    {
        FilePathList droppedFiles = LoadDroppedFiles();
        std::vector<std::string> droppedFilePaths;
        for (uint32_t i = 0; i < droppedFiles.count; i++) { droppedFilePaths.push_back(droppedFiles.paths[i]); }
        UnloadDroppedFiles(droppedFiles);
        if (droppedFilePaths.size() >= 1) { m_mediaPlayer.loadMedia(droppedFilePaths[0].c_str()); }
    }

    if (IsKeyPressed(KEY_SPACE))
    {
        if (m_mediaPlayer.isPaused()) { m_mediaPlayer.play(); }
        else
        {
            m_mediaPlayer.pause();
        }
    }
    else if (IsKeyPressed(KEY_LEFT)) { m_mediaPlayer.seek(-5.0); }
    else if (IsKeyPressed(KEY_RIGHT)) { m_mediaPlayer.seek(5.0); }
    else if (IsKeyPressed(KEY_UP)) { m_mediaPlayer.volume(5.0); }
    else if (IsKeyPressed(KEY_DOWN)) { m_mediaPlayer.volume(-5.0); }

    m_mediaPlayer.update();
    computeLayout();
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

    DrawRectangleRounded(m_elementRects.progress, 1.0f, 4, Colors::ProgressBase);

    float playedTime =
        m_mediaPlayer.currentTime() / (m_mediaPlayer.mediaProps().duration == 0.0 ? 1.0 : m_mediaPlayer.mediaProps().duration);
    Rectangle playedRect = m_elementRects.progress;
    playedRect.width     = playedRect.width * playedTime;
    DrawRectangleRounded(playedRect, 1.0f, 4, Colors::ProgressPlayed);
    DrawCircle(static_cast<int>(playedRect.x + playedRect.width), static_cast<int>(playedRect.y + playedRect.height * 0.5f), 7.0f,
               Colors::ScrubberHandle);
}

void MediaPlayerPage::computeLayout()
{
    const float screenW           = static_cast<float>(GetScreenWidth());
    const float screenH           = static_cast<float>(GetScreenHeight());

    m_elementRects.topBar         = {0.0f, 0.0f, screenW, PageConstants::TopBarHeight};
    m_elementRects.bottomBar      = {0.0f, screenH - PageConstants::BottomBarHeight, screenW, PageConstants::BottomBarHeight};

    m_elementRects.progress       = {PageConstants::Margin * 0.5f, m_elementRects.bottomBar.y, screenW - PageConstants::Margin,
                                     PageConstants::ProgressHeight};
    m_elementRects.progressHitBox = {m_elementRects.progress.x, m_elementRects.progress.y - PageConstants::ProgressHitBoxPadding,
                                     m_elementRects.progress.width,
                                     m_elementRects.progress.height + PageConstants::ProgressHitBoxPadding * 2.0f};

    float x                       = PageConstants::Margin;
    const float y                 = m_elementRects.bottomBar.y + PageConstants::ProgressHeight + 10.0f;
    const float btnSize           = PageConstants::IconSize + PageConstants::IconButtonPadding;
    m_elementRects.playButton     = {x, y, btnSize, btnSize};
    x += btnSize + PageConstants::ControlSpacing;

    m_elementRects.volumeIcon = {x, y, btnSize, btnSize};
    x += btnSize + 4.0f;

    m_elementRects.volumeSlider = {x, y + btnSize * 0.5f - 2.0f, PageConstants::VolumeSliderWidth, 4.0f};

    float rx                    = screenW - PageConstants::Margin - btnSize;
    m_elementRects.fullscreen   = {rx, y, btnSize, btnSize};
    rx -= btnSize + PageConstants::ControlSpacing;

    m_elementRects.settings = {rx, y, btnSize, btnSize};
}
}