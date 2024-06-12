#include "Checkbox.hpp"

START_NAMESPACE_DISTRHO

Checkbox::Checkbox(Widget *parent)
    : WAIVEWidget(parent),
      checked(true)
{
    loadSharedResources();
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
    strokeColor(stroke_color);
    if (checked)
        fillColor(accent_color);
    else
        fillColor(background_color);
    circle(r, r, r);
    fill();
    stroke();
    closePath();

    // label:
    beginPath();
    fillColor(text_color);
    fontSize(font_size);
    textAlign(Align::ALIGN_CENTER | Align::ALIGN_LEFT);
    text(r + 4.0f, height / 2.0f, label.c_str(), nullptr);
    closePath();
}

bool Checkbox::onMouse(const MouseEvent &ev)
{
    if (!isVisible() || !contains(ev.pos))
        return false;

    checked = !checked;
    if (callback != nullptr)
        callback->checkboxUpdated(this, checked);

    repaint();
    return true;
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

END_NAMESPACE_DISTRHO