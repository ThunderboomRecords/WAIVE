#ifndef SAMPLE_SLOT_HPP_INCLUDED
#define SAMPLE_SLOT_HPP_INCLUDED

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include <iostream>

START_NAMESPACE_DISTRHO

class SampleSlot : public NanoSubWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void sampleTriggered(int id) = 0;
        virtual void sampleRemoved(int id) = 0;
    };
    explicit SampleSlot(Widget *widget) noexcept;

    Color background_color, highlight_color;
    bool active;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;

private:
    DISTRHO_LEAK_DETECTOR(SampleSlot);
};

END_NAMESPACE_DISTRHO

#endif