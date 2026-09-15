#pragma once

#include "raylib.h"

#include <functional>

namespace Rayplayer
{
// ---------- MOUSE EVENT ----------

struct MouseEvent
{
    Vector2 pos{};
    bool pressed{false};
    bool down{false};
    bool released{false};

    MouseEvent() = default;
    MouseEvent(MouseButton button);
};

// ---------- WIDGET ----------

class Widget
{
  public:
    Widget()                                   = default;
    virtual ~Widget()                          = default;

    Widget(Widget &&other) noexcept            = delete;
    Widget &operator=(Widget &&other) noexcept = delete;

    Widget(const Widget &)                     = delete;
    Widget &operator=(const Widget &)          = delete;

    void bounds(Rectangle rect);
    [[nodiscard]] Rectangle bounds() const;

    void visible(bool value);
    [[nodiscard]] bool visible() const;

    virtual void update(const MouseEvent &event);
    virtual void draw() = 0;

  private:
    Rectangle m_bounds{};
    bool m_visible{true};
};

// ---------- BUTTON ----------

class Button final : public Widget
{
  public:
    std::function<void()> onClick;

    void icon(const Texture2D *texture);

    void iconSize(float size);
    [[nodiscard]] float iconSize() const;

    void update(const MouseEvent &event) override;
    void draw() override;

  private:
    const Texture2D *m_texture{};
    float m_iconSize{};
    bool m_isHovered{false};
};

// ---------- SLIDER ----------

class Slider final : public Widget
{
  public:
    std::function<void(double)> onValueChange;

    void value(double val);
    [[nodiscard]] double value() const;

    void handleColor(Color color);
    void fillColor(Color color);
    void trackColor(Color color);

    void update(const MouseEvent &event) override;
    void draw() override;

  private:
    Color m_handleColor{WHITE};
    Color m_fillColor{WHITE};
    Color m_trackColor{GRAY};

    double m_value{};
    bool m_isDragging{false};
    bool m_isHovered{false};

    Vector2 m_handleCenter{};
    float m_handleRadius{7.0f};
};
}