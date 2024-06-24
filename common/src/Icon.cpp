#include "Icon.hpp"

START_NAMESPACE_DISTRHO

Icon::Icon(Widget *widget) : WAIVEWidget(widget), hoverable(false), imageData(nullptr) {}

void Icon::setImageData(const uchar *data, uint dataSize, int width, int height, ImageFlags imageFlags)
{
    imageData = new WAIVEImage(this, data, dataSize, width, height, imageFlags);
}

void Icon::onNanoDisplay()
{
    if (imageData == nullptr)
        return;

    imageData->drawAt(0, 0, getWidth(), getHeight());
}

bool Icon::onMotion(const MotionEvent &ev)
{
    if (!hoverable)
        return false;

    hovered = contains(ev.pos);
    repaint();

    return hovered;
}

bool Icon::onMouse(const MouseEvent &ev) { return false; }

END_NAMESPACE_DISTRHO