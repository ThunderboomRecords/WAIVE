#ifndef PLAYHEAD_HPP_INCLUDED
#define PLAYHEAD_HPP_INCLUDED

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"

START_NAMESPACE_DISTRHO


class Playhead : public NanoSubWidget, public IdleCallback
{
public:
    explicit Playhead(Widget *widget) noexcept;

protected:
    void onNanoDisplay() override;
    void idleCallback() override;

private:

    float *progress;
    
    DISTRHO_LEAK_DETECTOR(Playhead);

    friend class WAIVEMidiUI;
};


END_NAMESPACE_DISTRHO

#endif