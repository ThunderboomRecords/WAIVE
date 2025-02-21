#ifndef KNOB_HPP_INCLUDED
#define KNOB_HPP_INCLUDED

#include "WAIVEWidget.hpp"
#include <fmt/core.h>

START_NAMESPACE_DISTRHO

class Knob : public WAIVEWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback() {};
        virtual void knobDragStarted(Knob *knob) = 0;
        virtual void knobDragFinished(Knob *knob, float value) = 0;
        virtual void knobValueChanged(Knob *knob, float value) = 0;
    };

    explicit Knob(Widget *widget) noexcept;

    void setCallback(Callback *cb);
    void setValue(float val, bool sendCallback = false) noexcept;
    float getValue() const noexcept;
    const std::string &getFormat() noexcept;
    void setRadius(float r, bool ignore_sf = false);
    void resizeToFit();

    float radius;
    float min, max;
    float gauge_width;
    std::string format;
    bool vertical;
    bool showValue;

    bool enabled;
    bool integer;

    std::string label;

    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;

protected:
    void onNanoDisplay() override;
    bool onScroll(const ScrollEvent &) override;
    void drawIndicator();
    void drawLabel();
    void drawValue();

private:
    Callback *callback;
    bool dragging_, mousedown_;
    float value_, tmp_p;
    float dragStart;
    bool sensitive;

    DISTRHO_LEAK_DETECTOR(Knob);
};

END_NAMESPACE_DISTRHO

#endif