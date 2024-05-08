#include "Label.hpp"

START_NAMESPACE_DISTRHO

Label::Label(Widget *parent, std::string text) noexcept
    : NanoSubWidget(parent),
      text_color(Color(40, 40, 40)),
      text_align(Align::ALIGN_BOTTOM),
      label_size(12.0f),
      label(text)
{
}

void Label::setText(std::string text)
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
    textAlign(text_align);
    fontSize(label_size);
    fontFaceId(font);

    Rectangle<float> bounds;
    textBounds(0, 0, label.c_str(), NULL, bounds);

    setWidth(bounds.getWidth());
    setHeight(bounds.getHeight());
}

void Label::onNanoDisplay()
{
    beginPath();
    fillColor(text_color);
    textAlign(text_align);
    fontSize(label_size);
    fontFaceId(font);
    text(0, getHeight(), label.c_str(), NULL);
    closePath();
}

END_NAMESPACE_DISTRHO
