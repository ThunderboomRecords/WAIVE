#ifndef KNOB_HPP_INCLUDED
#define KNOB_HPP_INCLUDED

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include <iostream>

START_NAMESPACE_DISTRHO

class Knob : public NanoSubWidget,
             public IdleCallback
{
public:
    class Callback {
        public:
            virtual ~Callback() {};
            virtual void knobDragStarted(Knob *knob) = 0;
            virtual void knobDragFinished(Knob *knob, float value) = 0;
            virtual void knobValueChanged(Knob *knob, float value) = 0;
    };

    explicit Knob(Widget *widget) noexcept;

    void setCallback(Callback *cb);
    void setValue(float val, bool sendCallback=false) noexcept;
    float getValue() noexcept;
    void idleCallback() override;

    float min, max;
    float gauge_width;
    Color foreground_color, background_color;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;

private:
    Callback *callback;
    bool dragging_;
    float value_, tmp_value_;
    int mouseY_;
    int last_mouse_y_;

    DISTRHO_LEAK_DETECTOR(Knob);
};



END_NAMESPACE_DISTRHO

#endif