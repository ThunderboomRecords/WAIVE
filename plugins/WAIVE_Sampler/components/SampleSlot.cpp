#include "SampleSlot.hpp"

START_NAMESPACE_DISTRHO

SampleSlot::SampleSlot(Widget *parent, DragDropManager *manager) noexcept
    : WidgetGroup(parent),
      DragDropWidget(manager),
      samplePlayer(nullptr),
      lastPlaying(PlayState::STOPPED),
      callback(nullptr),
      step(0.f),
      acceptingDrop(false),
      showMix(false),
      currentSampleId(-1)
{
    triggerBtn = new Button(parent);
    triggerBtn->setCallback(this);
    triggerBtn->setLabel("▶");
    triggerBtn->setSize(20, 20);
    triggerBtn->drawBackground = false;
    triggerBtn->description = "Trigger sample";
    triggerBtn->toFront();
    triggerBtn->setEnabled(false);

    sampleName = new Label(parent, "test");
    sampleName->setFont("Poppins-Regular", Poppins_Regular, Poppins_Regular_len);
    sampleName->setFontSize(12.0f);
    sampleName->text_align = Align::ALIGN_MIDDLE;

    clearBtn = new Button(parent);
    clearBtn->setCallback(this);
    clearBtn->setLabel("✕");
    clearBtn->setSize(20, 20);
    clearBtn->drawBackground = false;
    clearBtn->toFront();
    clearBtn->hide();
    clearBtn->description = "Remove sample.";

    midiSelect = new TextInput(parent);
    midiSelect->align = Align::ALIGN_CENTER;
    midiSelect->textType = TextInput::TextType::INTEGER;
    midiSelect->setFontSize(12.0f);
    midiSelect->setFont("Poppins-Regular", Poppins_Regular, Poppins_Regular_len);
    midiSelect->foreground_color = WaiveColors::light1;
    midiSelect->accent_color = WaiveColors::text;
    midiSelect->setSize(35, 20);
    midiSelect->setCallback(this);
    midiSelect->description = "Set MIDI note.";

    panKnob = new Knob(parent);
    panKnob->setCallback(this);
    panKnob->min = -1.f;
    panKnob->max = 1.f;
    panKnob->setValue(0.f);
    panKnob->showValue = false;
    panKnob->gauge_width = 1.0f * scale_factor;
    panKnob->accent_color = WaiveColors::text;
    panKnob->foreground_color = WaiveColors::light1;
    panKnob->setRadius(midiSelect->getHeight() / 2.f, true);
    panKnob->resizeToFit();
    panKnob->description = "Set playback pan.";
    panKnob->setVisible(false);

    gainKnob = new Knob(parent);
    gainKnob->setCallback(this);
    gainKnob->min = 0.f;
    gainKnob->max = 2.f;
    gainKnob->showValue = false;
    gainKnob->setValue(1.f);
    gainKnob->gauge_width = 1.0f * scale_factor;
    gainKnob->accent_color = WaiveColors::text;
    gainKnob->foreground_color = WaiveColors::light1;
    gainKnob->setRadius(midiSelect->getHeight() / 2.f, true);
    gainKnob->resizeToFit();
    gainKnob->description = "Set playback gain.";
    gainKnob->setVisible(false);

    contextMenu = new Menu(parent);
    contextMenu->addItem("Clear");
    contextMenu->addItem("Copy file path");
    contextMenu->setCallback(this);
    contextMenu->setDisplayNumber(2);
    contextMenu->setSize(100, 30);
    contextMenu->setFont("Poppins-Regular", Poppins_Regular, Poppins_Regular_len);
    contextMenu->calculateHeight();
    contextMenu->toFront();
    contextMenu->hide();

    addChildWidget(triggerBtn, {triggerBtn, this, Position::ON_TOP, Widget_Align::START, Widget_Align::CENTER, 5});
    addChildWidget(sampleName, {sampleName, triggerBtn, Position::RIGHT_OF, Widget_Align::CENTER, Widget_Align::START, 5});
    addChildWidget(midiSelect, {midiSelect, this, Position::ON_TOP, Widget_Align::END, Widget_Align::CENTER, 5});
    addChildWidget(clearBtn, {clearBtn, midiSelect, Position::LEFT_OF, Widget_Align::CENTER, Widget_Align::CENTER, 5});
    addChildWidget(panKnob, {panKnob, midiSelect, Position::LEFT_OF, Widget_Align::CENTER, Widget_Align::CENTER, 5});
    addChildWidget(gainKnob, {gainKnob, panKnob, Position::LEFT_OF, Widget_Align::CENTER, Widget_Align::CENTER, 5});
}

SampleSlot::~SampleSlot()
{
    if (samplePlayer != nullptr)
        samplePlayer->removeCallback(this);
}

void SampleSlot::setSamplePlayer(std::shared_ptr<SamplePlayer> sp)
{
    if (sp == nullptr)
        return;

    if (samplePlayer != nullptr)
        samplePlayer->removeCallback(this);

    samplePlayer = sp;
}

std::shared_ptr<SamplePlayer> SampleSlot::getSamplePlayer() const
{
    return samplePlayer;
}

void SampleSlot::dataAccepted(DragDropWidget *destination)
{
    if (callback != nullptr)
        callback->sampleSlotCleared(this, slotId);
    clearBtn->hide();
}

void SampleSlot::dataRejected(DragDropWidget *destination) {}

bool SampleSlot::onMouse(const MouseEvent &ev)
{
    double rightEdge = (showMix ? gainKnob->getLeft() : midiSelect->getLeft()) - getAbsoluteX();
    bool hovering = ev.pos.getX() >= sampleName->getLeft() - getAbsoluteX() &&
                    ev.pos.getX() <= rightEdge &&
                    ev.pos.getY() >= 0 &&
                    ev.pos.getY() <= getHeight();

    if (ev.button == kMouseButtonRight && hovering)
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
        if (ev.press)
        {
            if (hovering)
            {
                dragAction = DragAction::CLICKING;
                dragStart = ev.pos;
            }
        }
        else
        {
            if (hovering)
            {
                // std::cout << "SampleSlot::onMouse released" << std::endl;

                if (dragAction == DragAction::CLICKING && callback != nullptr)
                    callback->sampleSelected(this, slotId);

                if (dragDropManager->isDragging())
                {
                    // std::cout << "Accept drag event " << dragDropManager->getEvent().payload << std::endl;

                    DragDropWidget *source = dragDropManager->getEvent().source;

                    if (callback != nullptr && source != this)
                    {
                        int id = std::stoi(dragDropManager->getEvent().payload);
                        callback->sampleSlotLoadSample(this, slotId, id);
                        source->dataAccepted(this);
                        dragDropManager->dragDropEnd(this, true);
                    }
                    else
                    {
                        source->dataRejected(this);
                        dragDropManager->dragDropEnd(this, false);
                    }

                    dragDropManager->clearEvent();
                    acceptingDrop = false;
                }
            }

            dragAction = NONE;
        }
    }

    // std::cout << "SampleSlot : samplePlayer != nullPointer = " << (samplePlayer != nullptr) << ", samplePlayer->active = " << (samplePlayer->active) << ", samplePlayer->sampleInfo != nullptr = " << (samplePlayer->sampleInfo != nullptr) << std::endl;
    // std::cout << "  samplePlayer->sampleInfo->name = " << samplePlayer->sampleInfo->name << std::endl;

    return false;
}

bool SampleSlot::onMotion(const MotionEvent &ev)
{
    double rightEdge = (showMix ? gainKnob->getLeft() : midiSelect->getLeft()) - getAbsoluteX();
    bool hovering = ev.pos.getX() >= sampleName->getLeft() - getAbsoluteX() &&
                    ev.pos.getX() <= rightEdge &&
                    ev.pos.getY() >= 0 &&
                    ev.pos.getY() <= getHeight();

    if (hovering)
    {
        if (dragDropManager->isDragging())
        {
            acceptingDrop = true;
        }
        else if (samplePlayer->sampleInfo != nullptr && !clearBtn->isVisible() && !showMix)
            clearBtn->setVisible(true);
        else
            acceptingDrop = false;

        if (dragAction == DragAction::CLICKING)
        {
            if (std::abs(ev.pos.getX() - dragStart.getX()) > 5 || std::abs(ev.pos.getY() - dragStart.getY()) > 5)
            {
                dragAction = DragAction::SCROLLING;
                if (samplePlayer != nullptr && samplePlayer->sampleInfo != nullptr)
                {
                    dragDropManager->dragDropStart(this, fmt::format("{:d}", samplePlayer->sampleInfo->getId()), samplePlayer->sampleInfo->name);
                    dragDropManager->filepath = samplePlayer->sampleInfo->fullPath;
                }
            }
        }
    }
    else
    {
        acceptingDrop = false;

        if (clearBtn->isVisible())
            clearBtn->setVisible(false);
    }

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
            clearBtn->hide();
        }
        else if (item == 1)
        {
            std::string filename = samplePlayer->sampleInfo->fullPath;
            std::cout << "Copying to clipboard: " << filename << std::endl;
            SystemCopyToClipboard(filename);
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

    if (acceptingDrop)
    {
        beginPath();
        rect(1, 1, width - 2, height - 2);
        strokeColor(accent_color);
        strokeWidth(2);
        stroke();
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

void SampleSlot::textEntered(TextInput *textInput, const std::string &text)
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

    // if (samplePlayer != nullptr)
    //     samplePlayer->midi = val - 1;
    if (callback != nullptr)
        callback->sampleSlotMidiChanged(this, slotId, val - 1);
}

void SampleSlot::textInputChanged(TextInput *textInput, const std::string &text) {}

void SampleSlot::knobDragStarted(Knob *knob) {}

void SampleSlot::knobDragFinished(Knob *knob, float value) {}

void SampleSlot::knobValueChanged(Knob *knob, float value)
{
    if (knob == gainKnob)
    {
        if (callback != nullptr)
            callback->sampleSlotGainChanged(this, slotId, value);
    }
    else if (knob == panKnob)
    {
        if (callback != nullptr)
            callback->sampleSlotPanChanged(this, slotId, value);
    }
}

void SampleSlot::setMidiNumber(int midi, bool sendCallback)
{
    midiSelect->setText(fmt::format("{:d}", midi).c_str(), sendCallback);
}

void SampleSlot::setGain(float gain, bool sendCallback)
{
    gainKnob->setValue(gain, sendCallback);
}

void SampleSlot::setPan(float pan, bool sendCallback)
{
    panKnob->setValue(pan, sendCallback);
}

void SampleSlot::showMixControls(bool show)
{
    showMix = show;
    gainKnob->setVisible(show);
    panKnob->setVisible(show);

    if (show)
        sampleName->setWidth(WAIVELayout::measureHorizontal(sampleName, START, gainKnob, START));
    else
        sampleName->setWidth(WAIVELayout::measureHorizontal(sampleName, START, midiSelect, START));
}

void SampleSlot::sampleLoaded()
{
    triggerBtn->setEnabled(true);
    sampleName->setLabel(samplePlayer->sampleInfo->name.c_str());
    sampleName->resizeToFit();
    sampleName->setVisible(true);
    sampleName->setCenterY(triggerBtn->getCenterY());

    // getTopLevelWidget()->addIdleCallback(this);
}

void SampleSlot::sampleCleared()
{
    triggerBtn->setEnabled(false);
    sampleName->setLabel("");
    sampleName->resizeToFit();
    sampleName->setVisible(false);
    // getTopLevelWidget()->addIdleCallback(this);
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