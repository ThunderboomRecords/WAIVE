#include "SampleSlot.hpp"

START_NAMESPACE_DISTRHO

SampleSlot::SampleSlot(Widget *parent) noexcept
    : WidgetGroup(parent),
      samplePlayer(nullptr),
      lastPlaying(PlayState::STOPPED),
      callback(nullptr),
      currentSample(nullptr)
{
    triggerBtn = new Button(parent);
    triggerBtn->setCallback(this);
    triggerBtn->setLabel("▶");
    triggerBtn->setSize(20, 20);
    triggerBtn->drawBackground = false;
    triggerBtn->toFront();

    clearBtn = new Button(parent);
    clearBtn->setCallback(this);
    clearBtn->setLabel("✕");
    clearBtn->setSize(20, 20);
    clearBtn->drawBackground = false;
    clearBtn->toFront();
    clearBtn->hide();

    midiSelect = new DropDown(parent);
    for (int i = 0; i < 128; i++)
        midiSelect->addItem(fmt::format("{:d}", i).c_str());
    midiSelect->setDisplayNumber(6);
    midiSelect->setFontSize(16.0f);
    midiSelect->setFont("VG5000", VG5000, VG5000_len);
    midiSelect->setSize(35, 20);

    contextMenu = new Menu(parent);
    contextMenu->addItem("Clear");
    contextMenu->setCallback(this);
    contextMenu->setDisplayNumber(1);
    contextMenu->setSize(100, 30);
    contextMenu->setFont("VG5000", VG5000, VG5000_len);
    contextMenu->calculateHeight();
    contextMenu->toFront();
    contextMenu->hide();

    addChildWidget(triggerBtn, {triggerBtn, this, Position::ON_TOP, Widget_Align::START, Widget_Align::CENTER, 5});
    addChildWidget(midiSelect, {midiSelect, this, Position::ON_TOP, Widget_Align::END, Widget_Align::CENTER, 5});
    addChildWidget(clearBtn, {clearBtn, midiSelect, Position::LEFT_OF, Widget_Align::CENTER, Widget_Align::CENTER, 5});
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

        // return true;
    }
    else if (ev.button == kMouseButtonLeft)
    {
        if (callback != nullptr && samplePlayer != nullptr)
        {
            if (samplePlayer->sampleInfo != nullptr)
                callback->sampleSelected(this, slotId);
            else
                callback->sampleSlotCleared(this, slotId);
        }
    }

    return false;
}

bool SampleSlot::onMotion(const MotionEvent &ev)
{
    if (samplePlayer->sampleInfo != nullptr && contains(ev.pos) && !clearBtn->isVisible())
        clearBtn->setVisible(true);
    else if (clearBtn->isVisible() && (samplePlayer->sampleInfo == nullptr || !contains(ev.pos)))
        clearBtn->setVisible(false);

    return false;
}

void SampleSlot::onMenuItemSelection(Menu *menu, int item, const std::string &value)
{
    if (menu == contextMenu)
    {
        if (item == 0)
        {
            if (callback != nullptr)
                callback->sampleSlotCleared(this, slotId);
        }
    }
}

void SampleSlot::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    if (renderDebug)
    {
        beginPath();
        rect(0, 0, width, height);
        strokeColor(accent_color);
        stroke();
        closePath();
    }

    // sample info
    if (samplePlayer != nullptr && samplePlayer->active && samplePlayer->sampleInfo != nullptr)
    {
        if (currentSample != nullptr && currentSample == samplePlayer->sampleInfo)
        {
            beginPath();
            roundedRect(0, 0, width, height, 3 * scale_factor);
            fillColor(foreground_color);
            fill();
            closePath();
        }

        std::string info = samplePlayer->sampleInfo->name;
        float x = triggerBtn->getWidth() + 10;
        beginPath();
        fontSize(getFontSize());
        fillColor(text_color);
        textAlign(Align::ALIGN_MIDDLE);
        fontFaceId(font);
        text(x, height / 2, info.c_str(), nullptr);
        closePath();
    }
}

void SampleSlot::buttonClicked(Button *button)
{
    if (button == triggerBtn)
    {
        if (samplePlayer != nullptr)
            samplePlayer->state = PlayState::TRIGGERED;
    }
    else if (button == clearBtn)
    {
        if (callback != nullptr)
            callback->sampleSlotCleared(this, slotId);
        clearBtn->hide();
    }
}

void SampleSlot::dropdownSelection(DropDown *widget, int item)
{
    if (samplePlayer != nullptr)
        samplePlayer->midi = item;
}

void SampleSlot::setMidiNumber(int midi, bool sendCallback)
{
    midiSelect->setItem(midi, sendCallback);
}

void SampleSlot::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO