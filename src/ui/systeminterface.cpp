#include "systeminterface.hpp"
#include "log.hpp"

#include "raylib.h"

namespace Rayplayer
{
double SystemInterface::GetElapsedTime() { return GetTime(); }

bool SystemInterface::LogMessage(Rml::Log::Type type, const Rml::String &message)
{
    switch (type)
    {
    case Rml::Log::LT_WARNING: logger::warning("(RmlUi) {}", message); break;
    case Rml::Log::LT_ERROR: logger::error("(RmlUi) {}", message); break;
    default: logger::info("(RmlUi) {}", message); break;
    }
    return true;
}

void SystemInterface::SetMouseCursor(const Rml::String &cursor_name)
{
    if (cursor_name == "ns-resize") { ::SetMouseCursor(MOUSE_CURSOR_RESIZE_NS); }
    else if (cursor_name == "ew-resize") { ::SetMouseCursor(MOUSE_CURSOR_RESIZE_EW); }
    else if (cursor_name == "nesw-resize") { ::SetMouseCursor(MOUSE_CURSOR_RESIZE_NESW); }
    else if (cursor_name == "nwse-resize") { ::SetMouseCursor(MOUSE_CURSOR_RESIZE_NWSE); }
    else if (cursor_name == "move") { ::SetMouseCursor(MOUSE_CURSOR_RESIZE_ALL); }
    else
    {
        ::SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
}
}