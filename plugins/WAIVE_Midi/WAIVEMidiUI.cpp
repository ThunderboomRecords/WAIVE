#include "WAIVEMidiUI.hpp"


START_NAMESPACE_DISTRHO

WAIVEMidiUI::WAIVEMidiUI() : UI(UI_W, UI_H)
{
    plugin = static_cast<WAIVEMidi *>(getPluginInstancePointer());

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