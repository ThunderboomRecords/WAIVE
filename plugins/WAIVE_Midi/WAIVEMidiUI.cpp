#include "WAIVEMidiUI.hpp"


START_NAMESPACE_DISTRHO

WAIVEMidiUI::WAIVEMidiUI() : UI(UI_W, UI_H)
{
    plugin = static_cast<WAIVEMidi *>(getPluginInstancePointer());

    hbox_controls = new HBox(this);

    fThreshold = new VSlider(hbox_controls);
    fThreshold->setId(kThreshold);
    fThreshold->setSize(Size<uint>(20, 160));
    fThreshold->setCallback(this);
    fThreshold->gauge_width = 12.0f;
    fThreshold->max = 1.0f;
    fThreshold->foreground_color = Color(51, 51, 51);
    fThreshold->background_color = Color(255, 255, 255);
    fThreshold->marker_color = Color(0, 0, 0);

    hbox_controls->setAbsolutePos(10, 10);
    hbox_controls->setWidth(UI_W - 10);
    hbox_controls->padding = 10;
    hbox_controls->justify_content = HBox::Justify_Content::left;
    hbox_controls->addWidget(fThreshold);
    hbox_controls->positionWidgets();

    setGeometryConstraints(UI_W, UI_H, true, true);
}

WAIVEMidiUI::~WAIVEMidiUI() {}

void WAIVEMidiUI::parameterChanged(uint32_t index, float value)
{   
    switch(index)
    {
        case kThreshold:
            fThreshold->setValue(value);
        default:
            break;
    }

    repaint();
}


void WAIVEMidiUI::stateChanged(const char *key, const char *value)
{
    printf("WAIVEMidiUI::stateChanged()\n");

    repaint();
}

void WAIVEMidiUI::onNanoDisplay()
{
    float width = getWidth();
    float height = getHeight();

    beginPath();
    fillColor(Color(240, 240, 240));
    rect(0.0f, 0.0f, width, height);
    fill();
    closePath();

}

void WAIVEMidiUI::sliderDragStarted(Slider *slider){}

void WAIVEMidiUI::sliderDragFinished(Slider *slider, float value){}

void WAIVEMidiUI::sliderValueChanged(Slider *slider, float value)
{
    if(slider == fThreshold){
        setParameterValue(kThreshold, value);
    }
}

END_NAMESPACE_DISTRHO