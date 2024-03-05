#ifndef VSLIDER_HPP_INCLUDED
#define VSLIDER_HPP_INCLUDED

#include "Window.hpp"
#include "Widget.hpp"
#include "Slider.hpp"
#include "NanoVG.hpp"
#include <iostream>

START_NAMESPACE_DISTRHO

class VSlider : public Slider
{
public:
    explicit VSlider(Widget *widget) noexcept;

    void setCallback(Slider::Callback *cb);
    void setValue(float val, bool sendCallback=false) noexcept;
    float getValue() noexcept;

    float min, max;
    float gauge_width;
    Color foreground_color, background_color, marker_color;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;

private:
    Callback *callback;
    bool dragging_;
    float value_;

    DISTRHO_LEAK_DETECTOR(VSlider);
};



END_NAMESPACE_DISTRHO

#endif