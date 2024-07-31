#include "Knob.hpp"

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::Color;

Knob::Knob(Widget *parent) noexcept
    : WAIVEWidget(parent),
      dragging_(false),
      dragStart(0.0f),
      value_(0.0f),
      min(0.0f),
      max(1.0f),
      sensitive(false),
      callback(nullptr),
      format("{:.2f}"),
      label(""),
      enabled(true),
      radius(25.0f),
      integer(false)
{
    gauge_width = 8.f * scale_factor;
}

float Knob::getValue() noexcept
{
    return value_;
}

std::string Knob::getFormat() noexcept
{
    return format;
}

void Knob::setRadius(float r, bool ignore_sf)
{
    radius = r;
    if (!ignore_sf)
        radius *= scale_factor;
}

void Knob::resizeToFit()
{
    if (label.size() == 0)
    {
        setSize(2 * radius, 2 * radius, true);
        return;
    }

    fontFaceId(font);
    fontSize(getFontSize());
    Rectangle<float> bounds;
    textBounds(0, 0, label.c_str(), NULL, bounds);

    setSize(std::max(2 * radius, bounds.getWidth()), 2 * radius + bounds.getHeight() * 1.5, true);
}

void Knob::setValue(float val, bool sendCallback) noexcept
{
    if (val == value_)
        return;

    value_ = std::clamp(val, min, max);
    if (integer)
        value_ = std::floor(value_);

    if (sendCallback && callback != nullptr)
        callback->knobValueChanged(this, value_);

    repaint();
}

bool Knob::onMouse(const MouseEvent &ev)
{
    if (!isVisible() || ev.button != 1 || !enabled)
        return false;

    if (ev.press && contains(ev.pos))
    {

        dragging_ = true;
        dragStart = ev.pos.getY();
        tmp_p = (value_ - min) / (max - min);

        sensitive = (ev.mod == kModifierShift);

        if (callback != nullptr)
            callback->knobDragStarted(this);
        repaint();
    }
    else if (!ev.press && dragging_)
    {
        dragging_ = false;
        if (callback != nullptr)
            callback->knobDragFinished(this, value_);
    }
    else
        return false;

    return true;
}

bool Knob::onMotion(const MotionEvent &ev)
{
    if (!isVisible() || !dragging_ || !enabled)
        return false;

    const float height = getHeight();

    float scale = sensitive ? 400.0f : 150.0f;

    float d_y = ev.pos.getY() - dragStart;
    float d_p = d_y / scale;
    float p = tmp_p - d_p;

    float new_value = min + (max - min) * p;

    setValue(new_value, true);
    return true;
}

bool Knob::onScroll(const ScrollEvent &ev)
{
    if (!isVisible() || dragging_ || !contains(ev.pos) || !enabled)
        return false;

    sensitive = ev.mod == kModifierShift;

    float p = (getValue() - min) / (max - min);
    float scale = sensitive ? 0.01 : 0.03;

    float new_p = p + ev.delta.getY() * scale;
    float new_value = min + (max - min) * new_p;
    new_value = std::clamp(new_value, min, max);

    setValue(new_value, true);

    return true;
}

void Knob::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    if (renderDebug)
    {
        beginPath();
        strokeColor(accent_color);
        rect(0, 0, width, height);
        stroke();
        closePath();
    }

    const float center_x = width / 2.0f;
    const float center_y = height / 2.0f;

    float normValue = (getValue() - min) / (max - min);
    if (normValue < 0.0f)
        normValue = 0.0f;

    drawIndicator();

    beginPath();
    circle(center_x, center_y, radius - gauge_width - 2.0f);
    fillColor(foreground_color);
    fill();
    closePath();

    float angle = (0.75f + 1.5f * normValue) * M_PI;
    float x1 = cos(-angle) * (0.6f * radius) + center_x;
    float y1 = -sin(-angle) * (0.6f * radius) + center_y;
    float x2 = cos(-angle) * radius + center_x;
    float y2 = -sin(-angle) * radius + center_y;

    beginPath();
    strokeColor(accent_color);
    strokeWidth(gauge_width);
    moveTo(x1, y1);
    lineTo(x2, y2);
    stroke();
    closePath();

    drawLabel();
}

void Knob::drawIndicator()
{
    const float width = getWidth();
    const float height = getHeight();

    const float center_x = width / 2.0f;
    const float center_y = height / 2.0f;

    float normValue = (value_ - min) / (max - min);
    if (normValue < 0.0f)
        normValue = 0.0f;

    beginPath();
    strokeWidth(gauge_width);
    strokeColor(highlight_color);
    arc(center_x, center_y, radius - gauge_width / 2, 0.75f * M_PI, 0.25f * M_PI, NanoVG::Winding::CW);
    stroke();
    closePath();

    beginPath();
    strokeWidth(gauge_width);
    if (enabled)
        strokeColor(accent_color);
    else
    {
        float gray = 0.7f * (foreground_color.red + foreground_color.green + foreground_color.blue) / 3.0f;
        strokeColor(gray, gray, gray);
    }
    arc(
        center_x,
        center_y,
        radius - gauge_width / 2.0f,
        0.75f * M_PI,
        (0.75 + normValue * 1.5f) * M_PI,
        NanoVG::Winding::CW);
    stroke();
    closePath();
}

void Knob::drawLabel()
{
    if (label.size() == 0)
        return;

    beginPath();
    fillColor(text_color);
    fontFaceId(font);
    textAlign(Align::ALIGN_CENTER | Align::ALIGN_BOTTOM);
    fontSize(getFontSize());
    text(getWidth() / 2.0f, getHeight(), label.c_str(), nullptr);

    closePath();
}

void Knob::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO