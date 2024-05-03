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

    sample_display = new Waveform(this);
    sample_display->setSize(180, 80);
    sample_display->setAbsolutePos(UI_W - 10 - 180, 70 + 80 + 10);

    pitch = new Knob3D(this);
    pitch->setSize(50, 50);
    pitch->setAbsolutePos(UI_W - 10 - 180 - 100, 70 + 80 + 10);
    pitch->gauge_width = 6.0f;
    pitch->background_color = Color(190, 190, 190);
    pitch->foreground_color = Color(0, 160, 245);
    pitch->min = 0.25f;
    pitch->max = 4.0f;
    pitch->setValue(1.0f, false);
    pitch->setCallback(this);

    volume = new Knob3D(this);
    volume->setSize(50, 50);
    volume->setAbsolutePos(UI_W - 10 - 180 - 100 - 70, 70 + 80 + 10);
    volume->gauge_width = 6.0f;
    volume->background_color = Color(190, 190, 190);
    volume->foreground_color = Color(0, 160, 245);
    volume->min = 0.0f;
    volume->max = 2.0f;
    volume->setValue(1.0f, false);
    volume->setCallback(this);

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
    switch (index)
    {
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
    plugin->selectSample(&plugin->fSourceWaveform, selectionStart, selectionEnd);
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
    if (knob == volume)
    {
        setParameterValue(kSampleVolume, value);
    }

    value_indicator->setValue(knob->getValue());
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
    // fontFaceId(logo_font);
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
            waveform_display->calculateWaveform(&plugin->fSourceWaveform);
            break;
        case kSampleLoading:
            break;
        case kSampleUpdated:
            sample_display->calculateWaveform(&plugin->fSample);
            break;
        default:
            std::cout << "Unknown update: " << msg << std::endl;
            break;
        }
    }
}

END_NAMESPACE_DISTRHO