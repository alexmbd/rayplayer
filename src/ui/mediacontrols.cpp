#include "mediacontrols.hpp"

namespace Rayplayer
{
namespace
{
const char *g_clickIds[] = {"btn-play", "btn-volume", "btn-fullscreen"};
const char *g_mouseDownIds[] = {"progress-container", "volume-container"};
}

MediaControls::~MediaControls() {}

void MediaControls::init(Rml::ElementDocument *document, MediaPlayer *player)
{
    m_document = document;
    m_player   = player;
}

void MediaControls::ProcessEvent(Rml::Event &event) {}
}