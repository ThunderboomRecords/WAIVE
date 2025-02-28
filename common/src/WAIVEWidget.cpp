#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

WAIVEWidget::WAIVEWidget(Widget *widget, int flags) noexcept
    : NanoSubWidget(widget, flags),
      scale_factor(1.0f),
      background_color(WaiveColors::grey1),
      foreground_color(WaiveColors::grey2),
      highlight_color(WaiveColors::light1),
      accent_color(WaiveColors::accent1),
      text_color(WaiveColors::text),
      font_size(16.0f),
      font(0),
      renderDebug(false)
{
    loadSharedResources();
    scale_factor = getWindow().getScaleFactor();
}

void WAIVEWidget::above(NanoSubWidget *w, Widget_Align h_align, float padding)
{
    WAIVELayout::above(this, w, h_align, padding);
}

void WAIVEWidget::below(NanoSubWidget *w, Widget_Align h_align, float padding)
{
    WAIVELayout::below(this, w, h_align, padding);
}

void WAIVEWidget::leftOf(NanoSubWidget *w, Widget_Align v_align, float padding)
{
    WAIVELayout::leftOf(this, w, v_align, padding);
}

void WAIVEWidget::rightOf(NanoSubWidget *w, Widget_Align v_align, float padding)
{
    WAIVELayout::rightOf(this, w, v_align, padding);
}

void WAIVEWidget::onTop(NanoSubWidget *w, Widget_Align h_align, Widget_Align v_align, float h_padding, float v_padding)
{
    if (v_padding < 0.f)
        v_padding = h_padding;
    WAIVELayout::onTop(this, w, h_align, v_align, h_padding, v_padding);
}

void WAIVEWidget::setSize(uint width, uint height, bool ignore_sf)
{
    if (ignore_sf)
        NanoSubWidget::setSize(width, height);
    else
        NanoSubWidget::setSize(width * scale_factor, height * scale_factor);
}

void WAIVEWidget::setSize(const Size<uint> &size, bool ignore_sf)
{
    if (ignore_sf)
        NanoSubWidget::setSize(size.getWidth(), size.getHeight());
    else
        NanoSubWidget::setSize(size.getWidth() * scale_factor, size.getHeight() * scale_factor);
}

void WAIVEWidget::setFont(const char *name, const uchar *data, uint size)
{
    font = createFontFromMemory(name, data, size, false);
}

void WAIVEWidget::setFontSize(float size)
{
    font_size = size;
}

float WAIVEWidget::getFontSize(bool ignore_sf) const
{
    if (ignore_sf)
        return font_size;

    return font_size * scale_factor;
}

float WAIVEWidget::getTop() const
{
    return getAbsoluteY();
}

float WAIVEWidget::getBottom() const
{
    return getAbsoluteY() + getHeight();
}

float WAIVEWidget::getLeft() const
{
    return getAbsoluteX();
}

float WAIVEWidget::getRight() const
{
    return getAbsoluteX() + getWidth();
}

float WAIVEWidget::getCenterX() const
{
    return getAbsoluteX() + 0.5f * getWidth();
}

float WAIVEWidget::getCenterY() const
{
    return getAbsoluteY() + 0.5f * getHeight();
}

void WAIVEWidget::setLeft(float left)
{
    setAbsoluteX(left);
}

void WAIVEWidget::setRight(float right)
{
    setAbsoluteX(right - getWidth());
}

void WAIVEWidget::setTop(float top)
{
    setAbsoluteY(top);
}

void WAIVEWidget::setBottom(float bottom)
{
    setAbsoluteY(bottom - getHeight());
}

void WAIVEWidget::setCenterX(float centerX)
{
    setAbsoluteX(centerX - (getWidth() * 0.5f));
}

void WAIVEWidget::setCenterY(float centerY)
{
    setAbsoluteY(centerY - (getHeight() * 0.5f));
}

END_NAMESPACE_DISTRHO