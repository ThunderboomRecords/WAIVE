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

void WAIVEWidget::above(NanoSubWidget *w, Widget_Align h_align, int padding)
{
    Layout::above(this, w, h_align, padding);
}

void WAIVEWidget::below(NanoSubWidget *w, Widget_Align h_align, int padding)
{
    Layout::below(this, w, h_align, padding);
}

void WAIVEWidget::leftOf(NanoSubWidget *w, Widget_Align v_align, int padding)
{
    Layout::leftOf(this, w, v_align, padding);
}

void WAIVEWidget::rightOf(NanoSubWidget *w, Widget_Align v_align, int padding)
{
    Layout::rightOf(this, w, v_align, padding);
}

void WAIVEWidget::onTop(NanoSubWidget *w, Widget_Align h_align, Widget_Align v_align, int padding)
{
    Layout::onTop(this, w, h_align, v_align, padding);
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