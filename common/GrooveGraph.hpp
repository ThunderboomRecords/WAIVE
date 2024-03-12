#ifndef GROOVEGRAPH_HPP_INCLUDED
#define GROOVEGRAPH_HPP_INCLUDED


#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include <iostream>

#include "WAIVEMidi.hpp"

START_NAMESPACE_DISTRHO


class GrooveGraph : public NanoSubWidget
{
public:
    explicit GrooveGraph(Widget *widget) noexcept;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;

private:

    float (*fGroove)[48][3];
    
    DISTRHO_LEAK_DETECTOR(GrooveGraph);

    friend class WAIVEMidiUI;
};


END_NAMESPACE_DISTRHO

#endif