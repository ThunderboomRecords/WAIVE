#include "Panel.hpp"

START_NAMESPACE_DISTRHO

Panel::Panel(Widget *widget) : WAIVEWidget(widget) {}

void Panel::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(background_color);
    rect(0, 0, width, height);
    fill();
    closePath();
}

bool Panel::onMouse(const MouseEvent &ev)
{
    return contains(ev.pos);
}

bool Panel::onScroll(const ScrollEvent &ev)
{
    return contains(ev.pos);
}

bool Panel::onMotion(const MotionEvent &ev)
{
    return contains(ev.pos);
}

END_NAMESPACE_DISTRHO