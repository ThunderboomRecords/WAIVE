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
    Layout::above(this, w, h_align, padding);
}

void WAIVEWidget::below(NanoSubWidget *w, Widget_Align h_align, float padding)
{
    Layout::below(this, w, h_align, padding);
}

void WAIVEWidget::leftOf(NanoSubWidget *w, Widget_Align v_align, float padding)
{
    Layout::leftOf(this, w, v_align, padding);
}

void WAIVEWidget::rightOf(NanoSubWidget *w, Widget_Align v_align, float padding)
{
    Layout::rightOf(this, w, v_align, padding);
}

void WAIVEWidget::onTop(NanoSubWidget *w, Widget_Align h_align, Widget_Align v_align, float h_padding, float v_padding)
{
    if (v_padding < 0.f)
        v_padding = h_padding;
    Layout::onTop(this, w, h_align, v_align, h_padding, v_padding);
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

END_NAMESPACE_DISTRHO