#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

WAIVEWidget::WAIVEWidget(Widget *widget, int flags) noexcept
    : NanoSubWidget(widget, flags),
      scale_factor(1.0f),
      background_color(Color(200, 200, 200)),
      foreground_color(Color(120, 120, 120)),
      highlight_color(Color(Color(255, 255, 255), background_color, 0.5f)),
      stroke_color(Color(30, 30, 30)),
      accent_color(Color(0, 160, 245)),
      text_color(Color(10, 10, 10)),
      font_size(16.0f),
      font(0)
{
    scale_factor = getWindow().getScaleFactor();
}

void WAIVEWidget::setSize(uint width, uint height)
{
    NanoSubWidget::setSize(width * scale_factor, height * scale_factor);
}

void WAIVEWidget::setSize(const Size<uint> &size)
{
    NanoSubWidget::setSize(size.getWidth() * scale_factor, size.getHeight() * scale_factor);
}

void WAIVEWidget::fontSize(float size)
{
    NanoSubWidget::fontSize(size * scale_factor);
}

END_NAMESPACE_DISTRHO