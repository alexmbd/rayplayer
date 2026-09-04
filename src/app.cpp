#include "app.hpp"
#include "context.hpp"
#include "log.hpp"
#include "ui/mediaplayerpage.hpp"

#include "raylib.h"

namespace Rayplayer
{
namespace
{
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
    case LOG_ERROR: logger::error("(raylib) {}", buffer); break;
    case LOG_WARNING: logger::warning("(raylib) {}", buffer); break;
    case LOG_INFO:
    default: logger::info("(raylib) {}", buffer); break;
    }
}
}

int run()
{
    SetTraceLogCallback(raylibLog);
    SetTargetFPS(60);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIDDEN | FLAG_MSAA_4X_HINT);
    InitWindow(900, 600, "Rayplayer");

    details::Application app;

    ClearWindowState(FLAG_WINDOW_HIDDEN);
    while (!WindowShouldClose() && !context::shouldExit()) { app.update(); }

    if (context::shouldExit()) { logger::error("{}", context::lastError()); }

    CloseWindow();
    return context::shouldExit() ? -1 : 0;
}

namespace details
{
Application::Application()
{
    m_currentId          = m_nextId++;
    m_pages[m_currentId] = std::make_unique<MediaPlayerPage>();
}

void Application::update()
{
    // Omitted element check. Assume m_currentId is valid
    const auto &page = m_pages.at(m_currentId);
    page->update();

    BeginDrawing();
    page->draw();
    EndDrawing();
}
}
}