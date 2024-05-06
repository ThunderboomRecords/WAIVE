#ifndef WAIVE_SAMPLER_UI_HPP
#define WAIVE_SAMPLER_UI_HPP

#include <queue>

#include "DistrhoUI.hpp"
#include "NanoVG.hpp"
#include "Window.hpp"

#include "HBox.hpp"
#include "VBox.hpp"
#include "Knob.hpp"
#include "Knob3D.hpp"
#include "Waveform.hpp"
#include "SampleMap.hpp"
#include "SimpleButton.hpp"
#include "ValueIndicator.hpp"

#include "fonts.h"

#include "WAIVESampler.hpp"
#include "WAIVESamplerParams.h"

START_NAMESPACE_DISTRHO

const unsigned int UI_W = 840;
const unsigned int UI_H = 480;

class WAIVESamplerUI : public UI,
                       public Button::Callback,
                       public IdleCallback,
                       public Waveform::Callback,
                       public Knob::Callback,
                       public SampleMap::Callback
{
public:
    WAIVESamplerUI();
    ~WAIVESamplerUI();

protected:
    // void onFileSelected(const char* filename) override;
    void parameterChanged(uint32_t index, float value) override;
    void stateChanged(const char *key, const char *value) override;
    void onNanoDisplay() override;
    void uiScaleFactorChanged(const double scaleFactor) override;

    void buttonClicked(Button *button) override;
    void idleCallback() override;
    void waveformSelection(Waveform *waveform, uint selectionStart, uint selectionEnd) override;
    void knobDragStarted(Knob *knob) override;
    void knobDragFinished(Knob *knob, float value) override;
    void knobValueChanged(Knob *knob, float value) override;
    void mapSampleSelected(int id) override;

private:
    float fScale;
    double fScaleFactor;

    WAIVESampler *plugin;

    FontId logo_font;

    ValueIndicator *value_indicator;
    Button *open_button, *save_sample_button;
    Waveform *waveform_display, *sample_display;
    Knob3D *pitch, *volume, *ampAttack, *ampDecay, *ampSustain, *ampRelease;
    SampleMap *sample_map;
    HBox *ampADSRKnobs;

    DGL_NAMESPACE::FileBrowserOptions filebrowseropts;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WAIVESamplerUI);
};

UI *createUI()
{
    return new WAIVESamplerUI();
}

END_NAMESPACE_DISTRHO

#endif