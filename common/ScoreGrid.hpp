#ifndef SCOREGRID_HPP_INCLUDED
#define SCOREGRID_HPP_INCLUDED

#include "DistrhoUI.hpp"
#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"

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
    UI *ui;
    FontId label_font;

    float (*fScore)[16][9];
    int selected_16th, selected_ins;
    
    DISTRHO_LEAK_DETECTOR(ScoreGrid);

    friend class WAIVEMidiUI;
};


END_NAMESPACE_DISTRHO

#endif