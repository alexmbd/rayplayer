#include "context.hpp"
#include "log.hpp"
#include "mediaplayer/mediaplayer.hpp"
#include "ui/ui.hpp"

#include "rlgl.h"

#include <vector>

bool g_logRaylib = false;

void raylibLog(int logLevel, const char *text, va_list args)
{
    if (!g_logRaylib) { return; }

    va_list argsCopy;
    va_copy(argsCopy, args);
    int len = std::vsnprintf(nullptr, 0, text, argsCopy);
    va_end(argsCopy);

    if (len < 0) { return; }

    std::string buffer(len + 1, '\0');
    std::vsnprintf(buffer.data(), buffer.size(), text, args);
    buffer.resize(len);

    switch (logLevel)
    {
    case LOG_ERROR: Rayplayer::logger::error("(raylib) {}", buffer); break;
    case LOG_WARNING: Rayplayer::logger::warning("(raylib) {}", buffer); break;
    case LOG_INFO:
    default: Rayplayer::logger::info("(raylib) {}", buffer); break;
    }
}

Rectangle getLetterboxedDst(int videoW, int videoH, int winW, int winH)
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

int main()
{
    SetTraceLogCallback(raylibLog);
    SetTargetFPS(60);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIDDEN);
    InitWindow(900, 600, "Rayplayer");

    {
        Rayplayer::MediaPlayer mediaPlayer;
        mediaPlayer.init();

        Rayplayer::Ui ui;
        ui.init(&mediaPlayer);

        ClearWindowState(FLAG_WINDOW_HIDDEN);
        std::vector<std::string> droppedFilePaths;
        while (!WindowShouldClose() && !Rayplayer::context::shouldExit())
        {
            if (IsFileDropped())
            {
                FilePathList droppedFiles = LoadDroppedFiles();
                droppedFilePaths.clear();
                for (uint32_t i = 0; i < droppedFiles.count; i++) { droppedFilePaths.push_back(droppedFiles.paths[i]); }
                UnloadDroppedFiles(droppedFiles);
                if (droppedFilePaths.size() >= 1) { mediaPlayer.loadMedia(droppedFilePaths[0].c_str()); }
            }

            if (IsKeyPressed(KEY_SPACE))
            {
                if (mediaPlayer.isPaused()) { mediaPlayer.play(); }
                else
                {
                    mediaPlayer.pause();
                }
            }
            else if (IsKeyPressed(KEY_LEFT)) { mediaPlayer.seek(-5.0); }
            else if (IsKeyPressed(KEY_RIGHT)) { mediaPlayer.seek(5.0); }
            else if (IsKeyPressed(KEY_UP)) { mediaPlayer.volume(5.0); }
            else if (IsKeyPressed(KEY_DOWN)) { mediaPlayer.volume(-5.0); }

            mediaPlayer.update();
            ui.update();

            const RenderTexture2D &target           = mediaPlayer.texture();
            const Rayplayer::MediaProperties &props = mediaPlayer.mediaProps();
            Rectangle src{0, 0, static_cast<float>(target.texture.width), -static_cast<float>(target.texture.height)};
            Rectangle dst = getLetterboxedDst(props.videoWidth, props.videoHeight, GetScreenWidth(), GetScreenHeight());

            BeginDrawing();
            ClearBackground(BLACK);

            rlDisableColorBlend();
            DrawTexturePro(target.texture, src, dst, {0.0f, 0.0f}, 0.0f, WHITE);
            rlEnableColorBlend();

            ui.draw();

            DrawFPS(10, 10);
            EndDrawing();
        }
    }

    if (Rayplayer::context::shouldExit()) { Rayplayer::logger::error("{}", Rayplayer::context::lastError()); }

    CloseWindow();
    return 0;
}