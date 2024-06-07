#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

WAIVEWidget::WAIVEWidget(Widget *widget, int flags) noexcept
    : NanoSubWidget(widget, flags),
      scale_factor(1.0f)
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