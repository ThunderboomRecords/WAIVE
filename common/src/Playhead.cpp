#include "Playhead.hpp"

START_NAMESPACE_DISTRHO


Playhead::Playhead(Widget *parent) noexcept
    : NanoSubWidget(parent)
{

}

void Playhead::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    float x = (*progress) * width;

    strokeColor(220, 220, 220);
    beginPath();
    moveTo(x, 0);
    lineTo(x, height);
    closePath();
    stroke();
}

void Playhead::idleCallback()
{
    repaint();
}


END_NAMESPACE_DISTRHO