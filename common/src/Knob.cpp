#include "Knob.hpp"

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::Color;


Knob::Knob(Widget *parent) noexcept
    : NanoSubWidget(parent),
      dragging_(false),
      dragStart(0.0f),
      value_(0.0f),
      min(0.0f),
      max(1.0f),
      foreground_color(Color(200, 200, 200)),
      background_color(Color(40, 40, 40)),
      gauge_width(10.0f),
      sensitive(false),
      callback(nullptr)
{

}

float Knob::getValue() noexcept
{
    return value_;
}

void Knob::setValue(float val, bool sendCallback) noexcept
{
    if(val == value_) return;

    value_ = std::clamp(val, min, max);

    if(sendCallback && callback != nullptr)
    {
        callback->knobValueChanged(this, value_);
    }

    repaint();
}

bool Knob::onMouse(const MouseEvent &ev)
{
    if(!isVisible() || ev.button != 1) return false;

    if(ev.press && contains(ev.pos))
    {
        dragging_ = true;
        dragStart = ev.pos.getY();
        tmp_p = (value_ - min) / (max - min);

        sensitive = (ev.mod == kModifierShift);

        if(callback != nullptr)
        {
            callback->knobDragStarted(this);
        }
        repaint();
    } 
    else if(!ev.press && dragging_)
    {
        dragging_ = false;
    } 
    else 
    {
        return false;
    }

    return true;
}

bool Knob::onMotion(const MotionEvent &ev)
{
    if(!isVisible() || !dragging_) return false;

    const float height = getHeight();

    float scale = sensitive ? 200.0f : 100.0f;

    float d_y = ev.pos.getY() - dragStart;
    float d_p = d_y / scale;
    float p = tmp_p - d_p;

    float new_value = min + (max - min) * p;
    new_value = std::clamp(new_value, min, max);

    setValue(new_value, true);
    return true;
}

bool Knob::onScroll(const ScrollEvent &ev)
{
    if(!isVisible() || dragging_ || !contains(ev.pos)) return false;

    std::cout << "onScroll" << ev.delta.getY() << " " << std::endl;

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

    const float center_x = width/2.0f;
    const float center_y = height/2.0f;
    const float radius = std::min(center_x, center_y);

    float normValue = (value_ - min) / (max - min);
    if(normValue < 0.0f) normValue = 0.0f;

    beginPath();
    strokeWidth(gauge_width);
    strokeColor(background_color);
    arc(center_x, center_y, radius - gauge_width / 2, 0.75f * M_PI, 0.25f * M_PI, NanoVG::Winding::CW);
    stroke();
    closePath();

    beginPath();
    strokeWidth(gauge_width);
    strokeColor(foreground_color);
    arc(
        center_x,
        center_y,
        radius - gauge_width/2.0f,
        0.75f*M_PI,
        (0.75 + normValue*1.5f)*M_PI,
        NanoVG::Winding::CW
    );
    stroke();
    closePath();
}

void Knob::idleCallback()
{
    
}

void Knob::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO