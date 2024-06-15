#include "SampleSlot.hpp"

START_NAMESPACE_DISTRHO

SampleSlot::SampleSlot(Widget *parent) noexcept
    : WidgetGroup(parent),
      samplePlayer(nullptr),
      animation_step(1.0f),
      lastPlaying(PlayState::STOPPED),
      callback(nullptr)
{
    loadSharedResources();
    contextMenu = new Menu(parent);
    contextMenu->addItem("Clear");
    contextMenu->setCallback(this);
    contextMenu->setDisplayNumber(1);
    contextMenu->setSize(100, 30);
    contextMenu->setFont("VG5000", VG5000, VG5000_len);
    contextMenu->hide();
    contextMenu->calculateHeight();
}

void SampleSlot::setSamplePlayer(SamplePlayer *sp)
{
    samplePlayer = sp;
}

SamplePlayer *SampleSlot::getSamplePlayer() const
{
    return samplePlayer;
}

bool SampleSlot::onMouse(const MouseEvent &ev)
{
    if (!ev.press || !contains(ev.pos))
        return false;

    if (ev.button == kMouseButtonRight)
    {
        contextMenu->setAbsolutePos(
            ev.pos.getX() + getAbsoluteX() - 2,
            ev.pos.getY() + getAbsoluteY() - 8);
        contextMenu->toFront();
        contextMenu->show();

        return true;
    }
    else if (ev.button == kMouseButtonLeft)
    {
        if (callback != nullptr)
            callback->sampleSelected(this);
    }

    return false;
}

bool SampleSlot::onMotion(const MotionEvent &ev) { return false; }

void SampleSlot::onMenuItemSelection(Menu *menu, int item, const std::string &value)
{
    if (menu == contextMenu)
    {
        if (item == 0)
        {
            if (callback != nullptr)
                callback->sampleSlotCleared(this);
        }
    }
}

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

void SampleSlot::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO