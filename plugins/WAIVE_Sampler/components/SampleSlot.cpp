#include "SampleSlot.hpp"

START_NAMESPACE_DISTRHO

SampleSlot::SampleSlot(Widget *parent) noexcept
    : WidgetGroup(parent),
      samplePlayer(nullptr),
      lastPlaying(PlayState::STOPPED),
      callback(nullptr),
      step(0.f),
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

    midiSelect = new TextInput(parent);
    midiSelect->align = Align::ALIGN_CENTER;
    midiSelect->textType = TextInput::TextType::INTEGER;
    midiSelect->setFontSize(12.0f);
    midiSelect->setFont("Poppins-Regular", Poppins_Regular, Poppins_Regular_len);
    midiSelect->foreground_color = WaiveColors::light1;
    midiSelect->accent_color = WaiveColors::text;
    midiSelect->setSize(35, 20);
    midiSelect->setCallback(this);

    contextMenu = new Menu(parent);
    contextMenu->addItem("Clear");
    contextMenu->setCallback(this);
    contextMenu->setDisplayNumber(1);
    contextMenu->setSize(100, 30);
    contextMenu->setFont("Poppins-Regular", Poppins_Regular, Poppins_Regular_len);
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

        return true;
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

    // std::cout << "SampleSlot : samplePlayer != nullPointer = " << (samplePlayer != nullptr) << ", samplePlayer->active = " << (samplePlayer->active) << ", samplePlayer->sampleInfo != nullptr = " << (samplePlayer->sampleInfo != nullptr) << std::endl;
    // std::cout << "  samplePlayer->sampleInfo->name = " << samplePlayer->sampleInfo->name << std::endl;

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

        // Draw playing highlight
        beginPath();
        roundedRect(1, 1, width - 2, height - 2, 3 * scale_factor);
        strokeColor(.5f, .5f, .5f, step);
        strokeWidth(2.f);
        stroke();
        closePath();

        std::string info = samplePlayer->sampleInfo->name;
        float x = triggerBtn->getWidth() + 10;
        beginPath();
        fontSize(getFontSize());
        fontFaceId(font);
        fillColor(text_color);
        textAlign(Align::ALIGN_MIDDLE);
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

// void SampleSlot::dropdownSelection(DropDown *widget, int item)
// {
//     if (samplePlayer != nullptr)
//         samplePlayer->midi = item;
// }

void SampleSlot::textEntered(TextInput *textInput, std::string text)
{
    if (text.length() == 0)
    {
        textInput->undo();
        return;
    }

    errno = 0;
    char *endptr;
    long val = std::strtol(text.c_str(), &endptr, 10);

    if (endptr == text.c_str())
        return;
    else if (*endptr != '\0')
        return;

    bool clamped = false;
    if (val <= 0)
    {
        val = 1;
        clamped = true;
    }
    else if (val > 128)
    {
        val = 128;
        clamped = true;
    }

    if (clamped)
        textInput->setText(fmt::format("{:d}", val).c_str(), false);

    if (samplePlayer != nullptr)
        samplePlayer->midi = val - 1;

    // plugin->setMidiNote(textInput->getId(), (uint8_t)val);
}

void SampleSlot::textInputChanged(TextInput *textInput, std::string text) {}

void SampleSlot::setMidiNumber(int midi, bool sendCallback)
{
    midiSelect->setText(fmt::format("{:d}", midi).c_str(), sendCallback);
}

void SampleSlot::setCallback(Callback *cb)
{
    callback = cb;
}

void SampleSlot::idleCallback()
{
    float lastStep = step;

    if (samplePlayer == nullptr || !samplePlayer->active)
        step = 0.f;
    else if (samplePlayer->state == PlayState::STOPPED)
        step = 0.f;
    else
        step = (48000.f - (float)samplePlayer->ptr) / 48000.f;

    step = std::clamp(step, 0.f, 1.f);

    if (step != lastStep)
        repaint();
}

END_NAMESPACE_DISTRHO