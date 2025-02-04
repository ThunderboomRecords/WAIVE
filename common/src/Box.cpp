#include "Box.hpp"

START_NAMESPACE_DGL

Box::Box(Widget *widget) : WAIVEWidget(widget), radius(6.f) {}

void Box::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(background_color);
    roundedRect(0, 0, width, height, radius);
    fill();
    closePath();
}

END_NAMESPACE_DGL