#include "Checkbox.hpp"

START_NAMESPACE_DISTRHO

Checkbox::Checkbox(Widget *parent)
    : WAIVEWidget(parent),
      checked(true) {
      };

void Checkbox::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    // highlight background if hovering:
    if (focused)
    {
        beginPath();
        fillColor(highlight_color);
        rect(0, 0, width, height);
        fill();
        closePath();
    }

    // checkbox icon:
    float r = height / 2.0f - 1.0f;

    beginPath();
    strokeColor(checked ? accent_color : foreground_color);
    fillColor(checked ? accent_color : foreground_color);
    circle(r, r, r - 2.f);
    fill();
    stroke();
    closePath();

    // label:
    beginPath();
    fillColor(text_color);
    fontSize(getFontSize());
    textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_LEFT);
    text(2 * r + 4.0f, height / 2.0f, label.c_str(), nullptr);
    closePath();
}

bool Checkbox::onMouse(const MouseEvent &ev)
{
    if (!isVisible() || !contains(ev.pos))
        return false;

    if (ev.press && ev.button == kMouseButtonLeft)
    {
        checked = !checked;
        if (callback != nullptr)
            callback->checkboxUpdated(this, checked);

        repaint();
        return true;
    }

    return false;
}

bool Checkbox::onMotion(const MotionEvent &ev)
{
    if (!isVisible())
        return false;

    if (focused && !contains(ev.pos))
    {
        focused = false;
        repaint();
        return false;
    }
    else if (!focused && contains(ev.pos))
    {
        focused = true;
        repaint();
        return true;
    }

    return false;
}

void Checkbox::setChecked(bool value, bool sendCallback)
{
    checked = value;
    if (sendCallback && callback != nullptr)
        callback->checkboxUpdated(this, checked);
}

bool Checkbox::getChecked() const
{
    return checked;
}

void Checkbox::setCallback(Callback *cb)
{
    callback = cb;
}

void Checkbox::resize()
{
    float height = getFontSize() * 1.5f;
    float r = height / 2.0f - 1.0f;

    Rectangle<float> bounds;
    fontSize(getFontSize());
    textBounds(0, 0, label.c_str(), nullptr, bounds);
    printf("%.2f %.2f\n", bounds.getWidth(), bounds.getHeight());

    float width = bounds.getWidth() + 2 * r + 4.0f;
    setWidth(width);
    setHeight(height);
}

END_NAMESPACE_DISTRHO