#ifndef SAMPLE_SLOT_HPP_INCLUDED
#define SAMPLE_SLOT_HPP_INCLUDED

#include <iostream>

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"

#include <fmt/core.h>

#include "DropDown.hpp"
#include "SimpleButton.hpp"
#include "WAIVESampler.hpp"

using namespace fmt::v10;

START_NAMESPACE_DISTRHO

class SampleSlot : public NanoSubWidget,
                   public IdleCallback
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void sampleTriggered(int id) = 0;
        virtual void sampleRemoved(int id) = 0;
    };
    explicit SampleSlot(Widget *widget, DropDown *midi_select, Button *trigger_btn) noexcept;
    void setSamplePlayer(SamplePlayer *sp);
    void updateWidgetPositions();

    Color background_color, highlight_color;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    void idleCallback();

private:
    SamplePlayer *samplePlayer;
    DropDown *midi_number;
    Button *trigger_btn;

    DISTRHO_LEAK_DETECTOR(SampleSlot);

    PlayState lastPlaying = PlayState::STOPPED;

    float animation_step;
};

END_NAMESPACE_DISTRHO

#endif