#include "mediacontrols.hpp"
#include "context.hpp"

#include <format>

namespace Rayplayer
{
namespace
{
const char *g_clickIds[]     = {"btn-play", "btn-volume", "btn-fullscreen"};
const char *g_mouseDownIds[] = {"progress-container", "volume-container"};
}

MediaControls::~MediaControls() {}

void MediaControls::init(Rml::ElementDocument *document, MediaPlayer *player)
{
    if (context::shouldExit()) { return; }

    if (!document) { return context::requestExit("(MediaControls::init) 'document' passed is a nullptr"); }
    m_document = document;

    if (!player) { return context::requestExit("(MediaControls::init) 'player' passed is a nullptr"); }
    m_player = player;

    for (const char *id : g_clickIds)
    {
        Rml::Element *element = m_document->GetElementById(id);
        if (!element) { return context::requestExit(std::format("(MediaControls::init) Could not find element with id='{}'", id).c_str()); }
        element->AddEventListener("click", this);
    }

    for (const char *id : g_mouseDownIds)
    {
        Rml::Element *element = m_document->GetElementById(id);
        if (!element) { return context::requestExit(std::format("(MediaControls::init) Could not find element with id='{}'", id).c_str()); }
        element->AddEventListener("mousedown", this);
    }

    m_lastInputTime = GetTime();
}

void MediaControls::ProcessEvent(Rml::Event &event) {}
}