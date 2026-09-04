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
}
}