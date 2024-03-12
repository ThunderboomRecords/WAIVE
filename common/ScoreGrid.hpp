#ifndef SCOREGRID_HPP_INCLUDED
#define SCOREGRID_HPP_INCLUDED


#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include <iostream>

#include "WAIVEMidi.hpp"

START_NAMESPACE_DISTRHO


class ScoreGrid : public NanoSubWidget
{
public:
    explicit ScoreGrid(Widget *widget) noexcept;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;

private:

    float (*fScore)[16][9];
    
    DISTRHO_LEAK_DETECTOR(ScoreGrid);

    friend class WAIVEMidiUI;
};


END_NAMESPACE_DISTRHO

#endif