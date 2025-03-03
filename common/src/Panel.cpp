#include "Panel.hpp"

START_NAMESPACE_DISTRHO

Panel::Panel(Widget *widget)
    : WidgetGroup(widget),
      padding_h(24.f * scale_factor),
      padding_v(16.f * scale_factor),
      radius(6.f * scale_factor),
      expandable(false),
      expanded(false),
      expand_h(0.f),
      expand_v(0.f),
      hiddenWidgets(this),
      expand_down(true),
      expand_right(true)
{
    addChildWidget(&hiddenWidgets);
}

void Panel::getTitlAbsoluteBounds(DGL::Rectangle<float> &bounds)
{
    fontFaceId(font);
    fontSize(getFontSize());
    textAlign(ALIGN_TOP | ALIGN_LEFT);
    textBounds(
        padding_h + (expand_h - size_w) * expanded * !expand_right + getAbsoluteX(),
        padding_v + (expand_v - size_h) * expanded * !expand_down + getAbsoluteY(),
        (label + "  " + title).c_str(),
        nullptr,
        bounds);
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
    WAIVEWidget::setSize(width, height, ignore_sf);

    size_w = getWidth();
    size_h = getHeight();
}

void Panel::setSize(const Size<uint> &size, bool ignore_sf)
{
    WAIVEWidget::setSize(size, ignore_sf);

    size_w = size.getWidth();
    size_h = size.getHeight();
}

void Panel::expand()
{
    if (!expandable)
        return;

    expanded = true;
    WAIVEWidget::setSize(expand_h, expand_v, true);

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

    WAIVEWidget::setSize(size_w, size_h, true);

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

bool Panel::onMouse(const MouseEvent &ev)
{
    WAIVEWidget::onMouse(ev);

    return contains(ev.pos);
}

bool Panel::onMotion(const MotionEvent &ev)
{
    WAIVEWidget::onMotion(ev);

    return contains(ev.pos);
}

END_NAMESPACE_DISTRHO