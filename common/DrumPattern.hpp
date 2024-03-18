#ifndef DRUMPATTERN_HPP_INCLUDED
#define DRUMPATTERN_HPP_INCLUDED

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"

START_NAMESPACE_DISTRHO


class DrumPattern : public NanoSubWidget
{
public:
    explicit DrumPattern(Widget *widget) noexcept;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;

private:
    const int max_events[9] = {3, 7, 3, 3, 3, 4, 3, 2, 2};
    int s_map[9];

    float (*fDrumPattern)[16][30][3];
    
    DISTRHO_LEAK_DETECTOR(DrumPattern);

    friend class WAIVEMidiUI;
};



END_NAMESPACE_DISTRHO

#endif