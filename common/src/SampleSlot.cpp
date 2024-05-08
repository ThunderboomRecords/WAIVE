#include "SampleSlot.hpp"

START_NAMESPACE_DISTRHO

SampleSlot::SampleSlot(Widget *parent) noexcept
    : NanoSubWidget(parent),
      background_color(Color(200, 200, 200)),
      highlight_color(Color(30, 30, 30)),
      active(false)
{
}

bool SampleSlot::onMouse(const MouseEvent &ev) { return false; }
bool SampleSlot::onMotion(const MotionEvent &ev) { return false; }

void SampleSlot::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(background_color);
    roundedRect(0, 0, width, height, 6);
    fill();
    closePath();

    beginPath();
    if (active)
        strokeColor(highlight_color);
    else
        strokeColor(120, 120, 120);
    strokeWidth(3.f);
    roundedRect(1, 1, width - 2, height - 2, 3);
    stroke();
    closePath();

    // play button
}

END_NAMESPACE_DISTRHO