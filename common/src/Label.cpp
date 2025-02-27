#include "Label.hpp"

START_NAMESPACE_DISTRHO

Label::Label(Widget *parent, const std::string &text) noexcept
    : WAIVEWidget(parent),
      text_align(Align::ALIGN_BOTTOM),
      label(text),
      renderBackground(false),
      callback(nullptr)
{
}

void Label::setLabel(const std::string &text)
{
    label = text;
    repaint();
}

void Label::resizeToFit()
{
    if (label.length() == 0)
        return;

    fontSize(getFontSize());
    fontFaceId(font);
    textAlign(text_align);

    DGL::Rectangle<float> bounds;
    textBounds(0, 0, label.c_str(), NULL, bounds);

    setWidth(bounds.getWidth());
    setHeight(bounds.getHeight());
}

void Label::calculateHeight()
{
    if (label.length() == 0)
        return;

    fontSize(getFontSize());
    fontFaceId(font);
    textAlign(ALIGN_TOP | ALIGN_LEFT);

    const float width = getWidth();

    float bounds[4];
    textBoxBounds(0, 0, width, label.c_str(), NULL, bounds);

    std::cout << "Label::calculateHeight() ( width = " << width << ", font_size = " << getFontSize() << " ) " << bounds[0] << ", " << bounds[1] << ", " << bounds[2] << ", " << bounds[3] << std::endl;

    setHeight(bounds[3]);
}

void Label::setCallback(Callback *cb)
{
    callback = cb;
}

void Label::onNanoDisplay()
{
    if (label.length() == 0)
        return;

    if (renderDebug)
    {
        beginPath();
        strokeColor(accent_color);
        rect(0, 0, getWidth(), getHeight());
        stroke();
        closePath();
    }

    if (renderBackground)
    {
        beginPath();
        fillColor(background_color);
        rect(0, 0, getWidth(), getHeight());
        fill();
        closePath();
    }

    beginPath();
    fillColor(text_color);
    textAlign(ALIGN_TOP | ALIGN_LEFT);
    fontSize(getFontSize());
    fontFaceId(font);
    textBox(0, 0, getWidth(), label.c_str());
    // text(0, getHeight(), label.c_str());
    closePath();
}

bool Label::onMouse(const MouseEvent &ev)
{
    if (!ev.press || ev.button != kMouseButtonLeft || callback == nullptr || !contains(ev.pos))
        return false;

    callback->labelClicked(this);
    return false;
}

END_NAMESPACE_DISTRHO
