#include "VSlider.hpp"

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::Color;


VSlider::VSlider(Widget *parent) noexcept
    : Slider(parent)
{
    loadSharedResources();

    dragging_ = false;
    value_ = 0.0f;
    min = 0.0f;
    max = 1.0f;
    foreground_color = Color(255, 255, 255);
    background_color = Color(0, 0, 0);
    gauge_width = 6.0f;
}

float VSlider::getValue() noexcept
{
    return value_;
}

void VSlider::setValue(float val, bool sendCallback) noexcept
{
    if(val == value_) return;

    value_ = std::max(min, std::min(max, val));

    if(sendCallback && callback != nullptr)
    {
        callback->sliderValueChanged(this, value_);
    }
}

bool VSlider::onMouse(const MouseEvent &ev)
{
    if(!isVisible() || ev.button != 1) return false;

    if(ev.press)
    {
        if(!contains(ev.pos)) return false;

        dragging_ = true;

        if(callback != nullptr)
        {
            callback->sliderDragStarted(this);
        }
        repaint();
    } 
    else if(dragging_)
    {
        dragging_ = false;
        callback->sliderDragFinished(this, value_);
    }

    return false;
}

bool VSlider::onMotion(const MotionEvent &ev)
{
    if(!isVisible()) return false;
    
    if(!dragging_) return false;

    const float height = getHeight();

    float y_ = 1.0 - std::clamp(ev.pos.getY() / (double)height, 0.0, 1.0);
    float value = (max - min) * y_ + min;

    setValue(value, true);

    repaint();

    return false;
}

bool VSlider::onScroll(const ScrollEvent &ev)
{
    // TODO
    return false;
}

void VSlider::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    float normValue = (value_ - min) / (max - min);
    if(normValue < 0.0f) normValue = 0.0f;

    
    beginPath();
    fillColor(background_color);
    strokeColor(foreground_color);
    rect(0, 0, width, height);
    fill();
    stroke();
    closePath();

    beginPath();
    strokeColor(foreground_color);
    moveTo(width/2.0f, height);
    lineTo(width/2.0f, (1.0f - normValue)*height);
    stroke();
    closePath();

    beginPath();
    fillColor(marker_color);
    circle(width/2.0f, (1.0f - normValue)*height, 5);
    fill();
    closePath();
}

void VSlider::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO