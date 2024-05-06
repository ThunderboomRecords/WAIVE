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

    waveform_display = new Waveform(this);
    waveform_display->setSize(UI_W - 20, 80);
    waveform_display->setAbsolutePos(10, 70);
    waveform_display->selectable = true;
    waveform_display->setCallback(this);
    waveform_display->lineColor = Color(255, 255, 255);
    waveform_display->setWaveform(&plugin->fSourceWaveform);

    sample_display = new Waveform(this);
    sample_display->setSize(180, 80);
    sample_display->setAbsolutePos(UI_W - 10 - 180, 70 + 80 + 10);
    sample_display->setWaveform(&plugin->fSample);

    ampADSRKnobs = new HBox(this);
    ampADSRKnobs->setAbsolutePos(320, 160);
    ampADSRKnobs->setSize(300, 60);
    ampADSRKnobs->justify_content = HBox::Justify_Content::right;

    pitch = new Knob3D(this);
    pitch->setRadius(25.f);
    pitch->gauge_width = 6.0f;
    pitch->background_color = Color(190, 190, 190);
    pitch->foreground_color = Color(0, 160, 245);
    pitch->min = 0.25f;
    pitch->max = 4.0f;
    pitch->setValue(1.0f, false);
    pitch->setCallback(this);
    pitch->setId(kSamplePitch);
    pitch->label = "pitch";

    volume = new Knob3D(this);
    volume->setRadius(25.f);
    volume->gauge_width = 6.0f;
    volume->background_color = Color(190, 190, 190);
    volume->foreground_color = Color(0, 160, 245);
    volume->min = 0.0f;
    volume->max = 2.0f;
    volume->setValue(1.0f, false);
    volume->setCallback(this);
    volume->setId(kSampleVolume);
    volume->label = "volume";
    volume->font = logo_font;

    ampAttack = new Knob3D(this);
    ampAttack->setRadius(25.f);
    ampAttack->gauge_width = 6.0f;
    ampAttack->background_color = Color(190, 190, 190);
    ampAttack->foreground_color = Color(0, 160, 245);
    ampAttack->min = 0.0f;
    ampAttack->max = 500.0f;
    ampAttack->format = "{:.0f} ms";
    ampAttack->setValue(100.0f, false);
    ampAttack->setCallback(this);
    ampAttack->setId(kAmpAttack);
    ampAttack->label = "attack";

    ampDecay = new Knob3D(this);
    ampDecay->setRadius(25.f);
    ampDecay->gauge_width = 6.0f;
    ampDecay->background_color = Color(190, 190, 190);
    ampDecay->foreground_color = Color(0, 160, 245);
    ampDecay->min = 0.0f;
    ampDecay->max = 500.0f;
    ampDecay->format = "{:.0f} ms";
    ampDecay->setValue(100.0f, false);
    ampDecay->setCallback(this);
    ampDecay->setId(kAmpDecay);
    ampDecay->label = "decay";

    ampSustain = new Knob3D(this);
    ampSustain->setRadius(25.f);
    ampSustain->gauge_width = 6.0f;
    ampSustain->background_color = Color(190, 190, 190);
    ampSustain->foreground_color = Color(0, 160, 245);
    ampSustain->min = 0.0f;
    ampSustain->max = 1.0f;
    ampSustain->setValue(1.0f, false);
    ampSustain->setCallback(this);
    ampSustain->setId(kAmpSustain);
    ampSustain->label = "sustain";

    ampRelease = new Knob3D(this);
    ampRelease->setRadius(25.f);
    ampRelease->gauge_width = 6.0f;
    ampRelease->background_color = Color(190, 190, 190);
    ampRelease->foreground_color = Color(0, 160, 245);
    ampRelease->min = 0.0f;
    ampRelease->max = 500.0f;
    ampRelease->format = "{:.0f} ms";
    ampRelease->setValue(100.0f, false);
    ampRelease->setCallback(this);
    ampRelease->setId(kAmpRelease);
    ampRelease->label = "release";

    ampADSRKnobs->addWidget(pitch);
    ampADSRKnobs->addWidget(ampAttack);
    ampADSRKnobs->addWidget(ampDecay);
    ampADSRKnobs->addWidget(ampSustain);
    ampADSRKnobs->addWidget(ampRelease);
    ampADSRKnobs->addWidget(volume);
    ampADSRKnobs->positionWidgets();

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
    if (button == open_button)
    {
        requestStateFile("filename");
    }
    else if (button == save_sample_button)
    {
        plugin->addToLibrary();
    }
}

void WAIVESamplerUI::waveformSelection(Waveform *waveform, uint selectionStart, uint selectionEnd)
{
    plugin->selectWaveform(&plugin->fSourceWaveform, selectionStart, selectionEnd, true);
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
    if (knob == pitch)
    {
        setParameterValue(kSamplePitch, value);
    }
    value_indicator->hide();
    repaint();
}

void WAIVESamplerUI::knobValueChanged(Knob *knob, float value)
{
    // if (knob == volume)
    // {
    //     setParameterValue(kSampleVolume, value);
    // }

    if (knob == pitch)
        return;

    setParameterValue(knob->getId(), value);

    value_indicator->setValue(knob->getValue());
}

void WAIVESamplerUI::mapSampleSelected(int id)
{
    std::cout << "WAIVESamplerUI::mapSampleSelected " << id << std::endl;
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
            waveform_display->setSelection(plugin->fCurrentSample->sourceStart, plugin->fCurrentSample->sourceEnd, false);
            break;
        default:
            std::cout << "Unknown update: " << msg << std::endl;
            break;
        }
    }
}

END_NAMESPACE_DISTRHO