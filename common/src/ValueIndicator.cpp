#include "ValueIndicator.hpp"

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::Color;

ValueIndicator::ValueIndicator(Widget *parent) noexcept
    : WAIVEWidget(parent),
      fFormat("%.2f"),
      fValue(0.0f)
{
}

void ValueIndicator::setFormatString(std::string fmt)
{
    fFormat = fmt;
    repaint();
}

void ValueIndicator::setValue(float val)
{
    fValue = val;
    repaint();
}

void ValueIndicator::onNanoDisplay()
{
    std::string textString = fmt::format(fFormat, fValue);
    // std::cout << fFormat << " " << fValue << " -> " << textString << std::endl;

    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(background_color);
    roundedRect(0.0f, 0.0f, width, height, 4.0f);
    fill();
    closePath();

    beginPath();
    fillColor(text_color);
    textAlign(Align::ALIGN_CENTER | Align::ALIGN_MIDDLE);
    fontSize(getFontSize());
    fontFaceId(font);
    text(width / 2, height / 2, textString.c_str(), nullptr);
    closePath();
}

END_NAMESPACE_DISTRHO