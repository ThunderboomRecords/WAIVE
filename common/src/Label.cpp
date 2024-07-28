#include "Label.hpp"

START_NAMESPACE_DISTRHO

Label::Label(Widget *parent, std::string text) noexcept
    : WAIVEWidget(parent),
      text_align(Align::ALIGN_BOTTOM),
      label(text),
      callback(nullptr)
{
}

void Label::setLabel(std::string text)
{
    label = text;
    repaint();
}

void Label::setFont(const char *name, const uchar *data, uint size)
{
    font = createFontFromMemory(name, data, size, false);
    repaint();
}

void Label::resizeToFit()
{
    if (label.length() == 0)
        return;

    fontSize(getFontSize());
    fontFaceId(font);

    Rectangle<float> bounds;
    textBounds(0, 0, label.c_str(), NULL, bounds);

    setWidth(bounds.getWidth());
    setHeight(bounds.getHeight());
}

void Label::setCallback(Callback *cb)
{
    callback = cb;
}

void Label::onNanoDisplay()
{
    if (label.length() == 0)
        return;

    beginPath();
    fillColor(text_color);
    textAlign(text_align);
    fontSize(getFontSize());
    fontFaceId(font);
    text(0, getHeight(), label.c_str(), NULL);
    closePath();
}

bool Label::onMouse(const MouseEvent &ev)
{
    if (!ev.press || ev.button != kMouseButtonLeft || callback == nullptr || !contains(ev.pos))
        return false;

    callback->labelClicked(this);
    return true;
}

END_NAMESPACE_DISTRHO
