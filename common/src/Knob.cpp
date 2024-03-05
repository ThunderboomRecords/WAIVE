#include "Knob.hpp"

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::Color;


Knob::Knob(Widget *parent) noexcept
    : NanoSubWidget(parent)
{
    loadSharedResources();
    parent->getWindow().addIdleCallback(this);

    dragging_ = false;
    value_ = 0.0f;
    tmp_value_ = 0.0f;
    min = 0.0f;
    max = 1.0f;
    foreground_color = Color(255, 255, 255);
    background_color = Color(0, 0, 0);
    gauge_width = 6.0f;
}

float Knob::getValue() noexcept
{
    return value_;
}

void Knob::setValue(float val, bool sendCallback) noexcept
{
    if(val == value_) return;

    value_ = std::max(min, std::min(max, val));
    // float normValue = 0.0f;
    tmp_value_ = value_;

    // normValue = (value_ - min)/(max - min);

    if(sendCallback && callback != nullptr)
    {
        callback->knobValueChanged(this, value_);
    }
}

bool Knob::onMouse(const MouseEvent &ev)
{
    if(!isVisible() || ev.button != 1) return false;

    if(ev.press)
    {
        if(!contains(ev.pos)) return false;

        dragging_ = true;
        last_mouse_y_ = ev.pos.getY();

        if(callback != nullptr)
        {
            callback->knobDragStarted(this);
        }
        repaint();
        return false;
    } 
    else if(dragging_)
    {
        dragging_ = false;
    }

    return false;
}

bool Knob::onMotion(const MotionEvent &ev)
{
    if(!isVisible()) return false;
    

    if(!dragging_) return false;

    float d, value = 0.0f;
    d = 200.f;
    const int movY = last_mouse_y_ - ev.pos.getY();
    value = tmp_value_ - (float(min - max)/d * float(movY));
    
    // printf("min: %f  max: %f  value: %f\n", min, max, value);

    if(value < min) tmp_value_ = value = min;
    else if(value > max) tmp_value_ = value = max;

    setValue(value, true);
    last_mouse_y_ = ev.pos.getY();

    return false;
}

bool Knob::onScroll(const ScrollEvent &ev)
{
    return false;
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