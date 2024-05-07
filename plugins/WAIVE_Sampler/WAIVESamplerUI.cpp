#include "WAIVESamplerUI.hpp"
START_NAMESPACE_DISTRHO

WAIVESamplerUI::WAIVESamplerUI() : UI(UI_W, UI_H),
                                   fScaleFactor(getScaleFactor()),
                                   fScale(1.0f)
{
    plugin = static_cast<WAIVESampler *>(getPluginInstancePointer());

    logo_font = createFontFromMemory("VG5000", VG5000, VG5000_len, false);

    open_button = new Button(this);
    open_button->setLabel("Open");
    open_button->setFontScale(fScaleFactor);
    open_button->setBackgroundColor(Color(220, 220, 220));
    open_button->setLabelColor(Color(10, 10, 10));
    open_button->setSize(100, 50);
    open_button->setAbsolutePos(10, 10);
    open_button->setCallback(this);

    save_sample_button = new Button(this);
    save_sample_button->setLabel("add");
    save_sample_button->setFontScale(fScaleFactor);
    save_sample_button->setBackgroundColor(Color(220, 220, 220));
    save_sample_button->setLabelColor(Color(10, 10, 10));
    save_sample_button->setSize(70, 20);
    save_sample_button->setAbsolutePos(UI_W - 10 - 70, 70 + 80 + 10 + 80 + 10);
    save_sample_button->setCallback(this);

    play_button = new Button(this);
    play_button->setLabel("preview");
    play_button->setFontScale(fScaleFactor);
    play_button->setBackgroundColor(Color(220, 220, 220));
    play_button->setLabelColor(Color(10, 10, 10));
    play_button->setSize(70, 20);
    play_button->setAbsolutePos(UI_W - 10 - 180, 70 + 80 + 10 + 80 + 10);
    play_button->setCallback(this);

    waveform_display = new Waveform(this);
    waveform_display->setSize(UI_W - 20, 80);
    waveform_display->setAbsolutePos(10, 70);
    waveform_display->selectable = true;
    waveform_display->setCallback(this);
    waveform_display->lineColor = Color(255, 255, 255);
    waveform_display->setWaveform(&plugin->fSourceWaveform);
    waveform_display->waveformLength = &plugin->fSourceLength;

    sample_display = new Waveform(this);
    sample_display->setSize(180, 80);
    sample_display->setAbsolutePos(UI_W - 10 - 180, 70 + 80 + 10);
    sample_display->setWaveform(&plugin->fSample);
    sample_display->waveformLength = &plugin->fSampleLength;

    ampADSRKnobs = new HBox(this);
    ampADSRKnobs->setAbsolutePos(320, 160);
    ampADSRKnobs->setSize(300, 60);
    ampADSRKnobs->justify_content = HBox::Justify_Content::left;
    ampADSRKnobs->padding = 5;

    shapeKnobs = new HBox(this);
    shapeKnobs->setAbsolutePos(320, 240);
    shapeKnobs->setSize(300, 60);
    shapeKnobs->justify_content = HBox::Justify_Content::left;
    shapeKnobs->padding = 5;

    // Wave shaping
    pitch = createWAIVEKnob(this, kSamplePitch, "pitch", 0.25f, 4.f, 1.0f, logo_font);
    volume = createWAIVEKnob(this, kSampleVolume, "volume", 0.0f, 2.0f, 1.0f, logo_font);

    // Amp ADSR
    ampAttack = createWAIVEKnob(this, kAmpAttack, "attack", 0.0f, 500.0f, 10.0f, logo_font);
    ampAttack->format = "{:.0f}ms";

    ampDecay = createWAIVEKnob(this, kAmpDecay, "decay", 0.0f, 500.0f, 50.0f, logo_font);
    ampDecay->format = "{:.0f}ms";

    ampSustain = createWAIVEKnob(this, kAmpSustain, "sustain", 0.0f, 1.0f, 0.8f, logo_font);

    ampRelease = createWAIVEKnob(this, kAmpRelease, "release", 0.0f, 500.0f, 100.0f, logo_font);
    ampRelease->format = "{:.0f}ms";

    sustainLength = createWAIVEKnob(this, kSustainLength, "length", 0.0f, 500.0f, 100.f, logo_font);
    sustainLength->format = "{:.0f}ms";

    ampADSRKnobs->addWidget(ampAttack);
    ampADSRKnobs->addWidget(ampDecay);
    ampADSRKnobs->addWidget(ampSustain);
    ampADSRKnobs->addWidget(ampRelease);
    ampADSRKnobs->addWidget(sustainLength);
    ampADSRKnobs->positionWidgets();

    shapeKnobs->addWidget(sustainLength);
    shapeKnobs->addWidget(pitch);
    shapeKnobs->addWidget(volume);
    shapeKnobs->positionWidgets();

    sample_map = new SampleMap(this);
    sample_map->setSize(300, 300);
    sample_map->setAbsolutePos(10, 160);
    sample_map->allSamples = &plugin->fAllSamples;
    sample_map->setCallback(this);

    value_indicator = new ValueIndicator(this);
    value_indicator->setSize(70, 20);
    value_indicator->fontId = logo_font;
    value_indicator->hide();

    addIdleCallback(this);

    setGeometryConstraints(UI_W * fScaleFactor, UI_H * fScaleFactor, false, false);

    if (fScaleFactor != 1.0)
    {
        setSize(UI_W * fScaleFactor, UI_H * fScaleFactor);
    }
}

WAIVESamplerUI::~WAIVESamplerUI() {}

void WAIVESamplerUI::parameterChanged(uint32_t index, float value)
{
    std::cout << "WAIVESamplerUI::parameterChanged" << std::endl;
    switch (index)
    {
    case kSamplePitch:
        pitch->repaint();
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

END_NAMESPACE_DISTRHO