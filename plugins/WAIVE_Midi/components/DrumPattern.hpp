#ifndef DRUMPATTERN_HPP_INCLUDED
#define DRUMPATTERN_HPP_INCLUDED

#include "WAIVEWidget.hpp"
#include "Notes.hpp"

START_NAMESPACE_DISTRHO
class DrumPattern : public WAIVEWidget
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