#include "widgets.hpp"

#include <algorithm>

namespace Rayplayer
{
// ---------- MOUSE EVENT ----------

MouseEvent::MouseEvent(MouseButton button)
{
    pos      = GetMousePosition();
    pressed  = IsMouseButtonPressed(button);
    down     = IsMouseButtonDown(button);
    released = IsMouseButtonReleased(button);
}

// ---------- WIDGET ----------

void Widget::bounds(Rectangle rect) { m_bounds = rect; }
Rectangle Widget::bounds() const { return m_bounds; }
void Widget::visible(bool value) { m_visible = value; }
bool Widget::visible() const { return m_visible; }
void Widget::update(const MouseEvent &event) {}

// ---------- BUTTON ----------

void Button::icon(const Texture2D *texture) { m_texture = texture; }
void Button::iconSize(float size) { m_iconSize = size; }
float Button::iconSize() const { return m_iconSize; }

void Button::update(const MouseEvent &event)
{
    Widget::update(event);
    m_isHovered = visible() && CheckCollisionPointRec(event.pos, bounds());
    if (m_isHovered && event.pressed && onClick) { onClick(); }
}

void Button::draw()
{
    if (!visible()) { return; }

    const Rectangle rect = bounds();
    if (m_isHovered)
    {
        const Vector2 center{rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f};
        DrawCircleV(center, rect.width * 0.5f, Color{255, 255, 255, 40});
    }

    const Rectangle iconRect{rect.x + (rect.width - m_iconSize) * 0.5f, rect.y + (rect.height - m_iconSize) * 0.5f, m_iconSize, m_iconSize};
    if (m_texture != nullptr)
    {
        const Rectangle src{0, 0, static_cast<float>(m_texture->width), static_cast<float>(m_texture->height)};
        DrawTexturePro(*m_texture, src, iconRect, {0.0f, 0.0f}, 0.0f, WHITE);
    }
}

// ---------- SLIDER ----------

void Slider::value(double val) { m_value = std::clamp(val, 0.0, 1.0); }
double Slider::value() const { return m_value; }
void Slider::handleColor(Color color) { m_handleColor = color; }
void Slider::fillColor(Color color) { m_fillColor = color; }
void Slider::trackColor(Color color) { m_trackColor = color; }

void Slider::update(const MouseEvent &event)
{
    Widget::update(event);
    Rectangle rect = bounds();

    if (m_isDragging)
    {
        const float val = rect.width > 0.0f ? (event.pos.x - rect.x) / rect.width : 0.0f;
        m_value         = std::clamp(static_cast<double>(val), 0.0, 1.0);
        if (onValueChange) { onValueChange(m_value); }
        if (event.released) { m_isDragging = false; }
    }
    else
    {
        Rectangle hitRect{rect.x - 4.0f, rect.y - 4.0f, rect.width + 8.0f, rect.height + 8.0f};
        m_isHovered = CheckCollisionPointRec(event.pos, hitRect) || CheckCollisionPointCircle(event.pos, m_handleCenter, m_handleRadius);
        m_isHovered = m_isHovered && visible();
        if (m_isHovered && event.pressed) { m_isDragging = true; }
    }
}

void Slider::draw()
{
    if (!visible()) { return; }

    DrawRectangleRounded(bounds(), 1.0f, 4, m_trackColor);

    Rectangle fillRect = bounds();
    fillRect.width     = fillRect.width * static_cast<float>(m_value);
    DrawRectangleRounded(fillRect, 1.0f, 4, m_fillColor);

    Rectangle rect = bounds();
    m_handleCenter = {rect.x + fillRect.width, rect.y + rect.height * 0.5f};
    DrawCircleV(m_handleCenter, m_handleRadius, m_handleColor);
}
}