#include "SampleSlot.hpp"

START_NAMESPACE_DISTRHO

SampleSlot::SampleSlot(Widget *parent, DropDown *midi_select, Button *trigger_btn) noexcept
    : NanoSubWidget(parent),
      background_color(Color(200, 200, 200)),
      highlight_color(Color(30, 30, 30)),
      samplePlayer(nullptr),
      midi_number(midi_select),
      trigger_btn(trigger_btn),
      animation_step(1.0f)
{
    loadSharedResources();

    trigger_btn->fontSize(16.0f);
    trigger_btn->setLabel("▶");
    trigger_btn->setSize(20, 20);

    for (int i = 1; i < 128; i++)
        midi_number->addItem(fmt::format("{:d}", i).c_str());

    midi_number->setDisplayNumber(16);
    midi_number->font_size = 16.0f;
    midi_number->setSize(45, 20);
}

void SampleSlot::setSamplePlayer(SamplePlayer *sp)
{
    samplePlayer = sp;
}

void SampleSlot::updateWidgetPositions()
{
    const Rectangle<int> area = getAbsoluteArea();

    trigger_btn->setAbsoluteX(area.getX() + 4);
    trigger_btn->setAbsoluteY(area.getY() + (area.getHeight() - trigger_btn->getHeight()) / 2);

    midi_number->setAbsoluteX(area.getX() + area.getWidth() - midi_number->getWidth() - 4);
    midi_number->setAbsoluteY(area.getY() + (area.getHeight() - midi_number->getHeight()) / 2);
}

bool SampleSlot::onMouse(const MouseEvent &ev) { return false; }
bool SampleSlot::onMotion(const MotionEvent &ev) { return false; }

void SampleSlot::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    if (samplePlayer->active)
        strokeColor(Color(Color(120, 120, 120), highlight_color, animation_step));
    else
        strokeColor(180, 180, 180);
    strokeWidth(1.f);
    roundedRect(1, 1, width - 2, height - 2, 3);
    stroke();
    closePath();

    // sample info
    if (samplePlayer != nullptr && samplePlayer->active && samplePlayer->sampleInfo != nullptr)
    {
        std::string state;
        if (samplePlayer->state == PlayState::PLAYING)
            state = "playing";
        else
            state = "stopped";

        std::string info = fmt::format("{}: {}", samplePlayer->sampleInfo->name, state);

        beginPath();
        fontSize(14.0f);
        fillColor(Color(30, 30, 30));
        textAlign(Align::ALIGN_MIDDLE);
        fontFaceId(0);
        text(26, height / 2, info.c_str(), nullptr);
        closePath();
    }
}

void SampleSlot::idleCallback()
{
    if (samplePlayer == nullptr)
        return;

    bool needs_repaint = false;

    if (animation_step > 0.0f)
    {
        animation_step = std::max(0.0f, animation_step - 0.05f);
        needs_repaint = true;
    }

    if (lastPlaying != samplePlayer->state)
    {
        lastPlaying = samplePlayer->state;
        if (lastPlaying == PlayState::PLAYING)
            animation_step = 1.0f;

        needs_repaint = true;
    }

    if (needs_repaint)
        repaint();
}

END_NAMESPACE_DISTRHO