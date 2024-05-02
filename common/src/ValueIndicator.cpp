#include "ValueIndicator.hpp"

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::Color;

ValueIndicator::ValueIndicator(Widget *parent) noexcept
    : NanoSubWidget(parent),
      fFormat("%.2f"),
      fValue(0.0f),
      background_color(Color(30, 30, 30, 0.7f)),
      text_color(Color(200, 200, 200)),
      border_color(Color(200, 200, 200)),
      font_size(16.0f)
{
    loadSharedResources();
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
    rect(0.0f, 0.0f, width, height);
    fillColor(background_color);
    fill();
    closePath();

    beginPath();
    rect(1.0f, 1.0f, width-2.0f, height-2.0f);
    strokeColor(border_color);
    strokeWidth(1.0f);
    stroke();
    closePath();

    beginPath();
    fillColor(text_color);
    // fillColor(255, 255, 255);
    textAlign(Align::ALIGN_CENTER | Align::ALIGN_MIDDLE);
    fontSize(font_size);
    fontFaceId(fontId);
    text(width / 2, height / 2, textString.c_str(), nullptr);
    closePath();
}

END_NAMESPACE_DISTRHO