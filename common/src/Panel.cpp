#include "Panel.hpp"

START_NAMESPACE_DISTRHO

Panel::Panel(Widget *widget)
    : WidgetGroup(widget), padding(10.f), radius(10.f)
{
    // font_size = 18.f;
}

void Panel::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(background_color);
    roundedRect(0, 0, width, height, radius);
    fill();
    closePath();

    if (label.size() > 0)
    {
        beginPath();
        fillColor(text_color);
        fontFaceId(font);
        fontSize(getFontSize());
        textAlign(ALIGN_TOP | ALIGN_LEFT);
        text(padding, padding, label.c_str(), nullptr);
        closePath();
    }

    if (title.size() > 0)
    {
        beginPath();
        fillColor(text_color);
        fontFaceId(font);
        fontSize(getFontSize());
        textAlign(ALIGN_TOP | ALIGN_CENTER);
        text(width * 0.5f, padding, title.c_str(), nullptr);
        closePath();
    }
}

END_NAMESPACE_DISTRHO