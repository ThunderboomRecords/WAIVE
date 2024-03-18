#ifndef DRUMPATTERN_HPP_INCLUDED
#define DRUMPATTERN_HPP_INCLUDED

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include "Notes.hpp"

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
    std::vector<Note> *notes;
    std::map<uint8_t, int> midiToRow;
    
    DISTRHO_LEAK_DETECTOR(DrumPattern);

    friend class WAIVEMidiUI;
};


END_NAMESPACE_DISTRHO

#endif