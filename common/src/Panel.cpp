#include "Panel.hpp"

START_NAMESPACE_DISTRHO

Panel::Panel(Widget *widget)
    : WidgetGroup(widget),
      padding_h(24.f),
      padding_v(16.f),
      radius(6.f),
      expandable(false),
      expanded(false),
      hiddenWidgets(this),
      expand_down(true),
      expand_right(true)
{
    addChildWidget(&hiddenWidgets);
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
        text(
            padding_h + (expand_h - size_w) * expanded * !expand_right,
            padding_v + (expand_v - size_h) * expanded * !expand_down,
            (label + "  " + title).c_str(),
            nullptr);
        closePath();
    }
}

void Panel::setSize(uint width, uint height, bool ignore_sf)
{
    size_w = width;
    size_h = height;

    WAIVEWidget::setSize(width, height, ignore_sf);
}

void Panel::setSize(const Size<uint> &size, bool ignore_sf)
{
    size_w = size.getWidth();
    size_h = size.getHeight();

    WAIVEWidget::setSize(size, ignore_sf);
}

void Panel::expand()
{
    if (!expandable)
        return;

    expanded = true;
    WAIVEWidget::setSize(expand_h, expand_v);

    WAIVEWidget::toFront();
    WidgetGroup::toFront();

    if (!expand_right)
        setAbsoluteX(getAbsoluteX() - (expand_h - size_w));

    if (!expand_down)
        setAbsoluteY(getAbsoluteY() - (expand_v - size_h));

    hiddenWidgets.setVisible(true);
    // hiddenWidgets.toFront();
    repaint();
}

void Panel::collapse()
{
    if (!expandable)
        return;

    expanded = false;

    WAIVEWidget::setSize(size_w, size_h);

    if (!expand_right)
        setAbsoluteX(getAbsoluteX() + (expand_h - size_w));

    if (!expand_down)
        setAbsoluteY(getAbsoluteY() + (expand_v - size_h));

    hiddenWidgets.setVisible(false);
    repaint();
}

void Panel::toggle()
{
    if (expanded)
        collapse();
    else
        expand();
}

END_NAMESPACE_DISTRHO