#include "Popup.hpp"

START_NAMESPACE_DISTRHO

Popup::Popup(Widget *widget) : WAIVEWidget(widget)
{
}

void Popup::onNanoDisplay()
{
    const float width = getWidth();
    const float heihgt = getHeight();
}

END_NAMESPACE_DISTRHO