#pragma once

#include "mediaplayer/mediaplayer.hpp"

#include "RmlUi/Core.h"

namespace Rayplayer
{
class MediaControls final : public Rml::EventListener
{
  public:
    MediaControls() = default;
    ~MediaControls() override;

    MediaControls(MediaControls &&other) noexcept            = delete;
    MediaControls &operator=(MediaControls &&other) noexcept = delete;

    MediaControls(const MediaControls &)                     = delete;
    MediaControls &operator=(const MediaControls &)          = delete;

    void init(Rml::ElementDocument *document, MediaPlayer *player);

    void ProcessEvent(Rml::Event &event) override;

  private:
    Rml::ElementDocument *m_document{}; // Non-owning pointer to Rml::ElementDocument
    MediaPlayer *m_player{};            // Non-owning pointer to MediaPlayer

    double m_lastInputTime{0.0};

    bool m_isDraggingProgress{false};
    bool m_isDraggingVolume{false};
};
}