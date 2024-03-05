#ifndef WAIVE_MIDIUI_HPP
#define WAIVE_MIDIUI_HPP

#include "DistrhoPluginInfo.h"
#include "DistrhoUI.hpp"
#include "NanoVG.hpp"
#include "Window.hpp"

#include "VSlider.hpp"
#include "WAIVEMidi.hpp"


START_NAMESPACE_DISTRHO


const unsigned int UI_W = 800;
const unsigned int UI_H = 280;


class WAIVEMidiUI : public UI,
                    public Slider::Callback
{
public:
    WAIVEMidiUI();
    ~WAIVEMidiUI();

protected:
    void parameterChanged(uint32_t index, float value) override;
    void stateChanged(const char *key, const char *value) override;
    void onNanoDisplay() override;
    void sliderDragStarted(Slider *slider) override;
    void sliderDragFinished(Slider *slider, float value) override;
    void sliderValueChanged(Slider *slider, float value) override;

private:
    WAIVEMidi *plugin;
    VSlider *fThreshold;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WAIVEMidiUI);
};

UI *createUI()
{
    return new WAIVEMidiUI();
}

END_NAMESPACE_DISTRHO

#endif