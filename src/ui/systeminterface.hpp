#pragma once

#include "RmlUi/Core/SystemInterface.h"

namespace Rayplayer
{
class SystemInterface : public Rml::SystemInterface
{
  public:
    double GetElapsedTime() override;

    bool LogMessage(Rml::Log::Type type, const Rml::String &message) override;

    void SetMouseCursor(const Rml::String &cursor_name) override;
};
}