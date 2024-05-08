#include "WAIVESamplerUI.hpp"
START_NAMESPACE_DISTRHO

WAIVESamplerUI::WAIVESamplerUI() : UI(UI_W, UI_H),
                                   fScaleFactor(getScaleFactor()),
                                   fScale(1.0f)
{
    plugin = static_cast<WAIVESampler *>(getPluginInstancePointer());

    logo_font = createFontFromMemory("VG5000", VG5000, VG5000_len, false);

    sample_map = new SampleMap(this);
    sample_map->setSize(520, 300);
    sample_map->setAbsolutePos(10, 46);
    sample_map->allSamples = &plugin->fAllSamples;
    sample_map->setCallback(this);

    slots_container = new VBox(this);
    slots_container->setSize(UI_W - sample_map->getWidth() - 30, sample_map->getHeight());
    slots_container->padding = 2;
    slots_container->justify_content = VBox::Justify_Content::space_evenly;
    Layout::rightOf(slots_container, sample_map, Widget_Align::START, 10);
    createSampleSlots(this, &sampleSlots, slots_container, 8, 300.f / 8.f - 5.f);
    slots_container->positionWidgets();

    map_label = new Label(this, "sample map");
    map_label->setFont("VG5000", VG5000, VG5000_len);
    map_label->setSize(200, 20);
    map_label->label_size = 16.0f;
    Layout::above(map_label, sample_map, Widget_Align::START, 5);

    sample_controls = new Label(this, "sample controls");
    sample_controls->setFont("VG5000", VG5000, VG5000_len);
    sample_controls->setSize(200, 20);
    sample_controls->label_size = 16.0f;
    Layout::below(sample_controls, sample_map, Widget_Align::START, 5);

    waveform_display = new Waveform(this);
    waveform_display->setSize(UI_W - 20, 80);
    Layout::below(waveform_display, sample_controls, Widget_Align::START, 5);
    waveform_display->selectable = true;
    waveform_display->setCallback(this);
    waveform_display->lineColor = Color(255, 255, 255);
    waveform_display->setWaveform(&plugin->fSourceWaveform);
    waveform_display->waveformLength = &plugin->fSourceLength;

    open_button = new Button(this);
    open_button->setLabel("import source");
    open_button->setFontScale(fScaleFactor);
    open_button->setBackgroundColor(Color(40, 40, 40));
    open_button->setLabelColor(Color(200, 200, 200));
    open_button->setSize(100, 20);
    Layout::onTop(open_button, waveform_display, Widget_Align::END, Widget_Align::START, 2);
    open_button->setCallback(this);

    sample_display = new Waveform(this);
    sample_display->setSize(180, 80);
    Layout::below(sample_display, waveform_display, Widget_Align::END, 10.0f);
    sample_display->setWaveform(&plugin->fSample);
    sample_display->waveformLength = &plugin->fSampleLength;

    save_sample_button = new Button(this);
    save_sample_button->setLabel("add");
    save_sample_button->setFontScale(fScaleFactor);
    save_sample_button->setBackgroundColor(Color(220, 220, 220));
    save_sample_button->setLabelColor(Color(10, 10, 10));
    save_sample_button->setSize(70, 20);
    Layout::below(save_sample_button, sample_display, Widget_Align::END, 10.f);
    save_sample_button->setCallback(this);

    play_button = new Button(this);
    play_button->setLabel("preview");
    play_button->setFontScale(fScaleFactor);
    play_button->setBackgroundColor(Color(220, 220, 220));
    play_button->setLabelColor(Color(10, 10, 10));
    play_button->setSize(70, 20);
    Layout::below(play_button, sample_display, Widget_Align::START, 10.f);
    play_button->setCallback(this);

    ampADSRKnobs = new HBox(this);
    ampADSRKnobs->setSize(300, 60);
    ampADSRKnobs->justify_content = HBox::Justify_Content::right;
    ampADSRKnobs->padding = 5;

    shapeKnobs = new HBox(this);
    shapeKnobs->setSize(300, 60);
    shapeKnobs->justify_content = HBox::Justify_Content::right;
    shapeKnobs->padding = 5;

    // Wave shaping
    pitch = createWAIVEKnob(this, kSamplePitch, "pitch", 0.25f, 4.f, 1.0f, logo_font);
    volume = createWAIVEKnob(this, kSampleVolume, "volume", 0.0f, 2.0f, 1.0f, logo_font);

    // Amp ADSR
    ampAttack = createWAIVEKnob(this, kAmpAttack, "attack", 0.0f, 500.0f, 10.0f, logo_font);
    ampAttack->format = "{:.0f}ms";

    ampDecay = createWAIVEKnob(this, kAmpDecay, "decay", 0.0f, 500.0f, 50.0f, logo_font);
    ampDecay->format = "{:.0f}ms";

    ampSustain = createWAIVEKnob(this, kAmpSustain, "sustain", 0.0f, 1.0f, 0.7f, logo_font);

    ampRelease = createWAIVEKnob(this, kAmpRelease, "release", 0.0f, 500.0f, 100.0f, logo_font);
    ampRelease->format = "{:.0f}ms";

    sustainLength = createWAIVEKnob(this, kSustainLength, "length", 0.0f, 500.0f, 100.f, logo_font);
    sustainLength->format = "{:.0f}ms";

    shapeKnobs->addWidget(pitch);
    shapeKnobs->addWidget(volume);
    shapeKnobs->resizeToFit();
    Layout::leftOf(shapeKnobs, sample_display, Widget_Align::CENTER, 10.f);
    shapeKnobs->positionWidgets();

    ampADSRKnobs->addWidget(ampAttack);
    ampADSRKnobs->addWidget(ampDecay);
    ampADSRKnobs->addWidget(ampSustain);
    ampADSRKnobs->addWidget(ampRelease);
    ampADSRKnobs->addWidget(sustainLength);
    ampADSRKnobs->resizeToFit();
    Layout::leftOf(ampADSRKnobs, shapeKnobs, Widget_Align::CENTER, 10.f);
    ampADSRKnobs->positionWidgets();

    value_indicator = new ValueIndicator(this);
    value_indicator->setSize(70, 20);
    value_indicator->fontId = logo_font;
    value_indicator->hide();

    addIdleCallback(this);

    setGeometryConstraints(UI_W * fScaleFactor, UI_H * fScaleFactor, false, false);

    if (fScaleFactor != 1.0)
        setSize(UI_W * fScaleFactor, UI_H * fScaleFactor);
}

WAIVESamplerUI::~WAIVESamplerUI() {}

void WAIVESamplerUI::parameterChanged(uint32_t index, float value)
{
    switch (index)
    {
    case kSamplePitch:
        pitch->setValue(value, false);
        break;
    case kSampleVolume:
        volume->setValue(value, false);
        break;
    case kAmpAttack:
        ampAttack->setValue(value, false);
        break;
    case kAmpDecay:
        ampDecay->setValue(value, false);
        break;
    case kAmpSustain:
        ampSustain->setValue(value, false);
        break;
    case kAmpRelease:
        ampRelease->setValue(value, false);
        break;
    case kSustainLength:
        sustainLength->setValue(value, false);
        break;
    default:
        break;
    }

    repaint();
}

void WAIVESamplerUI::stateChanged(const char *key, const char *value)
{
    std::cout << "WAIVESamplerUI::stateChanged: " << key << " -> " << value << std::endl;

    if (strcmp(key, "filename") == 0)
    {
    }

    repaint();
}

void WAIVESamplerUI::buttonClicked(Button *button)
{
    LOG_LOCATION
    if (button == open_button)
        requestStateFile("filename");
    else if (button == save_sample_button)
        plugin->addToLibrary();
    else if (button == play_button)
        plugin->previewPlayer.state = PlayState::TRIGGERED;
}

void WAIVESamplerUI::waveformSelection(Waveform *waveform, uint selectionStart)
{
    plugin->selectWaveform(&plugin->fSourceWaveform, selectionStart, true);
}

void WAIVESamplerUI::knobDragStarted(Knob *knob)
{
    value_indicator->setAbsoluteX(knob->getAbsoluteX());
    value_indicator->setWidth(knob->getWidth());
    value_indicator->setAbsoluteY(knob->getAbsoluteY() + knob->getHeight());
    value_indicator->setFormatString(knob->getFormat());
    value_indicator->setValue(knob->getValue());
    value_indicator->show();
}

void WAIVESamplerUI::knobDragFinished(Knob *knob, float value)
{
    value_indicator->hide();
    repaint();
}

void WAIVESamplerUI::knobValueChanged(Knob *knob, float value)
{
    setParameterValue(knob->getId(), value);
    value_indicator->setValue(knob->getValue());
}

void WAIVESamplerUI::mapSampleSelected(int id)
{
    plugin->loadSample(id);
}

void WAIVESamplerUI::onNanoDisplay()
{
    float width = getWidth();
    float height = getHeight();

    beginPath();
    fillColor(Color(240, 240, 240));
    rect(0.0f, 0.0f, width, height);
    fill();
    closePath();

    beginPath();
    fillColor(Color(40, 40, 40));
    fontSize(32 * fScale * fScaleFactor);
    textAlign(Align::ALIGN_RIGHT | Align::ALIGN_TOP);

    fontFaceId(logo_font);
    text(width - 10 * fScale * fScaleFactor, 4 * fScale * fScaleFactor, "waive sampler", nullptr);
    closePath();
}

void WAIVESamplerUI::uiScaleFactorChanged(const double scaleFactor)
{
    fScaleFactor = scaleFactor;
}

void WAIVESamplerUI::idleCallback()
{
    std::queue<int> *updateQueue = &plugin->updateQueue;
    for (; !updateQueue->empty(); updateQueue->pop())
    {
        int msg = updateQueue->front();

        switch (msg)
        {
        case kSourceLoading:
            break;
        case kSourceLoaded:
            waveform_display->waveformNew();
            break;
        case kSourceUpdated:
            waveform_display->waveformUpdated();
            break;
        case kSampleLoading:
            break;
        case kSampleLoaded:
            sample_display->waveformNew();
            break;
        case kSampleUpdated:
            sample_display->waveformUpdated();
            break;
        case kSampleAdded:
            sample_map->repaint();
            break;
        case kParametersChanged:
            pitch->setValue(plugin->fSamplePitch, false);
            volume->setValue(plugin->fSampleVolume, false);
            ampAttack->setValue(plugin->fAmpADSRParams.attack, false);
            ampDecay->setValue(plugin->fAmpADSRParams.decay, false);
            ampSustain->setValue(plugin->fAmpADSRParams.sustain, false);
            ampRelease->setValue(plugin->fAmpADSRParams.release, false);
            sustainLength->setValue(plugin->fSustainLength, false);

            waveform_display->setSelection(plugin->fSampleStart, false);
            break;
        default:
            std::cout << "Unknown update: " << msg << std::endl;
            break;
        }
    }
}

// Helper functions to set up UI
Knob3D *createWAIVEKnob(
    WAIVESamplerUI *parent,
    Parameters param,
    std::string label,
    float min,
    float max,
    float value,
    UI::FontId font)
{
    Knob3D *knob = new Knob3D(parent);
    knob->setId(param);
    knob->label = label;
    knob->setRadius(25.f);
    knob->min = min;
    knob->max = max;
    knob->setValue(value);
    knob->gauge_width = 6.0f;
    knob->background_color = Color(190, 190, 190);
    knob->foreground_color = Color(0, 160, 245);
    knob->setCallback(parent);
    knob->font = font;

    return knob;
}

void createSampleSlots(
    Widget *parent,
    std::vector<SampleSlot *> *slots,
    VBox *container,
    int n,
    float height)
{
    float width = container->getWidth();
    for (int i = 0; i < n; i++)
    {
        SampleSlot *slot = new SampleSlot(parent);
        slot->setSize(width, height);

        float hue = ((float)n - i) / (n + 2);
        slot->highlight_color = Color::fromHSL(hue, 0.8f, 0.7f);
        container->addWidget(slot);
        slots->push_back(slot);
    }
}

END_NAMESPACE_DISTRHO