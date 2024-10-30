#ifndef SAMPLE_SLOT_HPP_INCLUDED
#define SAMPLE_SLOT_HPP_INCLUDED

#include <fmt/core.h>

#include "WAIVESampler.hpp"
#include "WidgetGroup.hpp"
#include "Menu.hpp"
#include "SimpleButton.hpp"
#include "DropDown.hpp"

using namespace fmt::v11;

START_NAMESPACE_DISTRHO

class SampleSlot : public WidgetGroup,
                   public Menu::Callback,
                   Button::Callback,
                   DropDown::Callback
{
public:
    class Callback
    {
    public:
        virtual ~Callback() {};
        virtual void sampleSelected(SampleSlot *slot, int slotId) = 0;
        virtual void sampleSlotCleared(SampleSlot *slot, int slotId) = 0;
    };
    explicit SampleSlot(Widget *widget) noexcept;

    void setCallback(Callback *cb);
    void setSamplePlayer(SamplePlayer *sp);
    void setMidiNumber(int midi, bool sendCallback);
    SamplePlayer *getSamplePlayer() const;
    int slotId;
    std::shared_ptr<SampleInfo> currentSample;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;

    void onMenuItemSelection(Menu *menu, int item, const std::string &value) override;
    void buttonClicked(Button *button) override;
    void dropdownSelection(DropDown *widget, int item) override;

private:
    Callback *callback;

    SamplePlayer *samplePlayer;
    PlayState lastPlaying;

    Menu *contextMenu;
    Button *triggerBtn, *clearBtn;
    DropDown *midiSelect;

    DISTRHO_LEAK_DETECTOR(SampleSlot);
};

END_NAMESPACE_DISTRHO

#endif