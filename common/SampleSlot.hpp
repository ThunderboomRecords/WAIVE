#ifndef SAMPLE_SLOT_HPP_INCLUDED
#define SAMPLE_SLOT_HPP_INCLUDED

#include <fmt/core.h>

#include "Menu.hpp"
#include "WAIVESampler.hpp"
#include "WidgetGroup.hpp"

using namespace fmt::v10;

START_NAMESPACE_DISTRHO

class SampleSlot : public WidgetGroup,
                   public IdleCallback,
                   public Menu::Callback
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void sampleTriggered(SampleSlot *slot) = 0;
        virtual void sampleSlotCleared(SampleSlot *slot) = 0;
    };
    explicit SampleSlot(Widget *widget) noexcept;

    void setCallback(Callback *cb);
    void setSamplePlayer(SamplePlayer *sp);
    SamplePlayer *getSamplePlayer() const;
    int slotId;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    void idleCallback();

    void onMenuItemSelection(Menu *menu, int item, const std::string &value) override;

private:
    Callback *callback;

    SamplePlayer *samplePlayer;
    PlayState lastPlaying;

    Menu *contextMenu;

    float animation_step;

    DISTRHO_LEAK_DETECTOR(SampleSlot);
};

END_NAMESPACE_DISTRHO

#endif