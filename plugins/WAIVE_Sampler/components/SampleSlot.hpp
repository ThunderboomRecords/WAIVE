#ifndef SAMPLE_SLOT_HPP_INCLUDED
#define SAMPLE_SLOT_HPP_INCLUDED

#include <fmt/core.h>

#include "WAIVESampler.hpp"
#include "WidgetGroup.hpp"
#include "Menu.hpp"
#include "TextInput.hpp"
#include "SimpleButton.hpp"
#include "DragDrop.hpp"

#include "SamplePlayer.hpp"

#include "WAIVEUtils.hpp"

START_NAMESPACE_DISTRHO

class SampleSlot : public WidgetGroup,
                   public Menu::Callback,
                   public DragDropWidget,
                   public IdleCallback,
                   public SamplePlayerCallback,
                   Button::Callback,
                   TextInput::Callback
{
public:
    class Callback
    {
    public:
        virtual ~Callback() {};
        virtual void sampleSelected(SampleSlot *slot, int slotId) = 0;
        virtual void sampleSlotCleared(SampleSlot *slot, int slotId) = 0;
        virtual void sampleSlotLoadSample(SampleSlot *slot, int slotId, int sampleId) = 0;
    };
    explicit SampleSlot(Widget *widget, DragDropManager *manager) noexcept;
    ~SampleSlot() override;

    void setCallback(Callback *cb);
    void idleCallback() override;

    void setSamplePlayer(std::shared_ptr<SamplePlayer> sp);
    void setMidiNumber(int midi, bool sendCallback);
    std::shared_ptr<SamplePlayer> getSamplePlayer() const;
    int slotId, currentSampleId;

    void sampleLoaded() override;
    void sampleCleared() override;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;

    void onMenuItemSelection(Menu *menu, int item, const std::string &value) override;
    void buttonClicked(Button *button) override;
    void textEntered(TextInput *textInput, const std::string &text) override;
    void textInputChanged(TextInput *textInput, const std::string &text) override;

    void dataAccepted(DragDropWidget *destination) override;
    void dataRejected(DragDropWidget *destination) override;

private:
    Callback *callback;

    std::shared_ptr<SamplePlayer> samplePlayer;
    PlayState lastPlaying;

    Menu *contextMenu;
    Button *triggerBtn, *clearBtn;
    TextInput *midiSelect;

    float step;
    DGL::Point<double> dragStart;
    DragAction dragAction;
    bool acceptingDrop;

    DISTRHO_LEAK_DETECTOR(SampleSlot);
};

END_NAMESPACE_DISTRHO

#endif