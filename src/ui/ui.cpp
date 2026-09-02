#include "ui.hpp"
#include "context.hpp"

#include <format>

namespace Rayplayer
{
Ui::~Ui()
{
    m_mediaControls.~MediaControls();
    m_document->Close();

    if (!Rml::RemoveContext(m_context->GetName()))
    {
        context::requestExit(std::format("(Ui::~Ui) Invalid context name='{}'", m_context->GetName()).c_str());
    }

    Rml::Shutdown();
}

void Ui::init(MediaPlayer *player)
{
    if (context::shouldExit()) { return; }

    Rml::SetRenderInterface(&m_renderinterface);
    Rml::SetSystemInterface(&m_systeminterface);

    if (!Rml::Initialise()) { return context::requestExit("(Ui::init) 'Rml::Initialise' failed"); }

    m_context = Rml::CreateContext("main", Rml::Vector2i(GetScreenWidth(), GetScreenHeight()));
    if (!m_context) { return context::requestExit("(Ui::init) Could not create context using 'Rml::CreateContext'"); }

    const char *fontFilePath = "resources/fonts/Roboto/static/Roboto-Regular.ttf";
    if (!Rml::LoadFontFace(fontFilePath))
    {
        return context::requestExit(std::format("(Ui::init) Failed to load '{}'", fontFilePath).c_str());
    }

    const char *rmlFilePath = "resources/ui/mediacontrols.rml";
    m_document              = m_context->LoadDocument(rmlFilePath);
    if (!m_document) { return context::requestExit(std::format("(Ui::init) Failed to load '{}'", rmlFilePath).c_str()); }

    m_document->Show();

    m_mediaControls.init(m_document, player);
}

void Ui::update()
{
    if (context::shouldExit()) { return; }

    // Window Events
    if (IsWindowResized()) { m_context->SetDimensions(Rml::Vector2i(GetScreenWidth(), GetScreenHeight())); }

    // Modifiers
    int keyboardModifiers = 0;
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) { keyboardModifiers |= Rml::Input::KM_CTRL; }
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) { keyboardModifiers |= Rml::Input::KM_SHIFT; }
    if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) { keyboardModifiers |= Rml::Input::KM_ALT; }

    // Handle Mouse Inputs
    m_context->ProcessMouseMove(GetMouseX(), GetMouseY(), keyboardModifiers);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { m_context->ProcessMouseButtonDown(0, keyboardModifiers); }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) { m_context->ProcessMouseButtonUp(0, keyboardModifiers); }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) { m_context->ProcessMouseButtonDown(1, keyboardModifiers); }
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) { m_context->ProcessMouseButtonUp(1, keyboardModifiers); }

    const float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) { m_context->ProcessMouseWheel(Rml::Vector2f(0.0f, -wheel), keyboardModifiers); }

    // Handle Keyboard Inputs
    for (int key = KEY_SPACE; key <= KEY_KB_MENU; key++)
    {
        const Rml::Input::KeyIdentifier rmlKey = raylibToRmlKey(key);
        if (rmlKey == Rml::Input::KI_UNKNOWN) { continue; }

        if (IsKeyPressed(key)) { m_context->ProcessKeyDown(rmlKey, keyboardModifiers); }
        if (IsKeyReleased(key)) { m_context->ProcessKeyUp(rmlKey, keyboardModifiers); }
    }

    if (IsKeyPressed(KEY_ENTER) && keyboardModifiers == 0)
    {
        m_context->ProcessKeyDown(Rml::Input::KI_RETURN, 0);
        m_context->ProcessTextInput("\n");
        m_context->ProcessKeyUp(Rml::Input::KI_RETURN, 0);
    }

    int ch = GetCharPressed();
    while (ch != 0)
    {
        m_context->ProcessTextInput(static_cast<Rml::Character>(ch));
        ch = GetCharPressed();
    }

    m_context->Update();
}

void Ui::draw()
{
    if (context::shouldExit()) { return; }

    rlSetBlendMode(RL_BLEND_ALPHA_PREMULTIPLY);
    m_context->Render();
    rlSetBlendMode(RL_BLEND_ALPHA);
}
}