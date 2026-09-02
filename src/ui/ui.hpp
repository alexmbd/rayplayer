#pragma once

#include "mediacontrols.hpp"
#include "renderinterface.hpp"
#include "systeminterface.hpp"

namespace Rayplayer
{
class Ui final
{
  public:
    Ui() = default;
    ~Ui();

    Ui(Ui &&other) noexcept            = delete;
    Ui &operator=(Ui &&other) noexcept = delete;

    Ui(const Ui &)                     = delete;
    Ui &operator=(const Ui &)          = delete;

    void init(MediaPlayer *player);
    void update();
    void draw();

  private:
    RenderInterface m_renderinterface;
    SystemInterface m_systeminterface;

    Rml::Context *m_context;
    Rml::ElementDocument *m_document;

    MediaControls m_mediaControls;
};

[[nodiscard]] inline Rml::Input::KeyIdentifier raylibToRmlKey(int key)
{
    using namespace Rml::Input;

    switch (key)
    {
    // Letters
    case KEY_A: return KI_A;
    case KEY_B: return KI_B;
    case KEY_C: return KI_C;
    case KEY_D: return KI_D;
    case KEY_E: return KI_E;
    case KEY_F: return KI_F;
    case KEY_G: return KI_G;
    case KEY_H: return KI_H;
    case KEY_I: return KI_I;
    case KEY_J: return KI_J;
    case KEY_K: return KI_K;
    case KEY_L: return KI_L;
    case KEY_M: return KI_M;
    case KEY_N: return KI_N;
    case KEY_O: return KI_O;
    case KEY_P: return KI_P;
    case KEY_Q: return KI_Q;
    case KEY_R: return KI_R;
    case KEY_S: return KI_S;
    case KEY_T: return KI_T;
    case KEY_U: return KI_U;
    case KEY_V: return KI_V;
    case KEY_W: return KI_W;
    case KEY_X: return KI_X;
    case KEY_Y: return KI_Y;
    case KEY_Z: return KI_Z;

    // Digits (top row)
    case KEY_ZERO: return KI_0;
    case KEY_ONE: return KI_1;
    case KEY_TWO: return KI_2;
    case KEY_THREE: return KI_3;
    case KEY_FOUR: return KI_4;
    case KEY_FIVE: return KI_5;
    case KEY_SIX: return KI_6;
    case KEY_SEVEN: return KI_7;
    case KEY_EIGHT: return KI_8;
    case KEY_NINE: return KI_9;

    // Numpad
    case KEY_KP_0: return KI_NUMPAD0;
    case KEY_KP_1: return KI_NUMPAD1;
    case KEY_KP_2: return KI_NUMPAD2;
    case KEY_KP_3: return KI_NUMPAD3;
    case KEY_KP_4: return KI_NUMPAD4;
    case KEY_KP_5: return KI_NUMPAD5;
    case KEY_KP_6: return KI_NUMPAD6;
    case KEY_KP_7: return KI_NUMPAD7;
    case KEY_KP_8: return KI_NUMPAD8;
    case KEY_KP_9: return KI_NUMPAD9;
    case KEY_KP_DECIMAL: return KI_DECIMAL;
    case KEY_KP_DIVIDE: return KI_DIVIDE;
    case KEY_KP_MULTIPLY: return KI_MULTIPLY;
    case KEY_KP_SUBTRACT: return KI_SUBTRACT;
    case KEY_KP_ADD: return KI_ADD;
    case KEY_KP_ENTER: return KI_NUMPADENTER;

    // Function keys
    case KEY_F1: return KI_F1;
    case KEY_F2: return KI_F2;
    case KEY_F3: return KI_F3;
    case KEY_F4: return KI_F4;
    case KEY_F5: return KI_F5;
    case KEY_F6: return KI_F6;
    case KEY_F7: return KI_F7;
    case KEY_F8: return KI_F8;
    case KEY_F9: return KI_F9;
    case KEY_F10: return KI_F10;
    case KEY_F11: return KI_F11;
    case KEY_F12: return KI_F12;

    // Navigation / editing
    case KEY_LEFT: return KI_LEFT;
    case KEY_RIGHT: return KI_RIGHT;
    case KEY_UP: return KI_UP;
    case KEY_DOWN: return KI_DOWN;
    case KEY_HOME: return KI_HOME;
    case KEY_END: return KI_END;
    case KEY_PAGE_UP: return KI_PRIOR;
    case KEY_PAGE_DOWN: return KI_NEXT;
    case KEY_INSERT: return KI_INSERT;
    case KEY_DELETE: return KI_DELETE;
    case KEY_BACKSPACE: return KI_BACK;
    case KEY_TAB: return KI_TAB;
    case KEY_ENTER: return KI_RETURN;
    case KEY_ESCAPE: return KI_ESCAPE;
    case KEY_SPACE: return KI_SPACE;

    // Modifiers
    case KEY_LEFT_SHIFT: return KI_LSHIFT;
    case KEY_RIGHT_SHIFT: return KI_RSHIFT;
    case KEY_LEFT_CONTROL: return KI_LCONTROL;
    case KEY_RIGHT_CONTROL: return KI_RCONTROL;
    case KEY_LEFT_ALT: return KI_LMENU;
    case KEY_RIGHT_ALT: return KI_RMENU;
    case KEY_LEFT_SUPER: return KI_LMETA;
    case KEY_RIGHT_SUPER: return KI_RMETA;
    case KEY_CAPS_LOCK: return KI_CAPITAL;
    case KEY_NUM_LOCK: return KI_NUMLOCK;
    case KEY_SCROLL_LOCK: return KI_SCROLL;

    // Punctuation
    case KEY_APOSTROPHE: return KI_OEM_7;
    case KEY_COMMA: return KI_OEM_COMMA;
    case KEY_MINUS: return KI_OEM_MINUS;
    case KEY_PERIOD: return KI_OEM_PERIOD;
    case KEY_SLASH: return KI_OEM_2;
    case KEY_SEMICOLON: return KI_OEM_1;
    case KEY_EQUAL: return KI_OEM_PLUS;
    case KEY_LEFT_BRACKET: return KI_OEM_4;
    case KEY_BACKSLASH: return KI_OEM_5;
    case KEY_RIGHT_BRACKET: return KI_OEM_6;
    case KEY_GRAVE: return KI_OEM_3;

    default: return KI_UNKNOWN;
    }
}
}