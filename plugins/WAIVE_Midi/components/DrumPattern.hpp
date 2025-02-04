#ifndef DRUMPATTERN_HPP_INCLUDED
#define DRUMPATTERN_HPP_INCLUDED

#include <mutex>

#include "WAIVEWidget.hpp"
#include "Notes.hpp"

START_NAMESPACE_DISTRHO
class DrumPattern : public WAIVEWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback() {};
        virtual void onDrumPatternClicked(DrumPattern *widget, int instrument, int sixteenth) = 0;
        virtual void onDrumPatternScrolled(DrumPattern *widget, std::shared_ptr<Note> note, float deltaY) = 0;
        virtual void onDrumPatternNoteMoved(DrumPattern *widget, std::shared_ptr<Note> note, uint32_t tick) = 0;
        virtual void onNoteDeleted(DrumPattern *widget, std::shared_ptr<Note> note) = 0;
    };
    explicit DrumPattern(Widget *widget) noexcept;

    void setCallback(Callback *cb);

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;

private:
    enum DragState
    {
        NONE,
        CLICKING,
        DRAGGING
    };

    std::shared_ptr<Note> findHoveredNote(const DGL::Point<double> &pos);

    Callback *callback;

    std::mutex *noteMtx;
    std::vector<std::shared_ptr<Note>> *notes;
    std::vector<Trigger> *triggersGenerated, *triggersUser;

    std::map<uint8_t, int> midiToRow;
    std::shared_ptr<Note> selectedNote, hoveredNote;
    uint32_t newTick;

    DragState dragState;
    DGL::Point<double> dragStart, hover;
    bool hovering, hoveringGrid;

    friend class WAIVEMidiUI;
    DISTRHO_LEAK_DETECTOR(DrumPattern);
};

END_NAMESPACE_DISTRHO

#endif