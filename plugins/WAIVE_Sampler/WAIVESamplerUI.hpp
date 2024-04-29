#ifndef WAIVE_SAMPLER_UI_HPP
#define WAIVE_SAMPLER_UI_HPP

#include <queue>

#include "DistrhoUI.hpp"
#include "NanoVG.hpp"
#include "Window.hpp"

#include "HBox.hpp"
#include "VBox.hpp"
#include "SimpleButton.hpp"
#include "Waveform.hpp"

#include "fonts.h"

#include "WAIVESampler.hpp"
#include "WAIVESamplerParams.h"



START_NAMESPACE_DISTRHO

const unsigned int UI_W = 840;
const unsigned int UI_H = 380;


class WAIVESamplerUI : public UI,
                       public Button::Callback,
                       public IdleCallback
{
public:
    WAIVESamplerUI();
    ~WAIVESamplerUI();

protected:
    void parameterChanged(uint32_t index, float value) override;
    void stateChanged(const char *key, const char *value) override;
    void onNanoDisplay() override;
    void uiScaleFactorChanged(const double scaleFactor) override;

    void buttonClicked(Button *button) override;
    void idleCallback() override;

private:
    float fScale;
    double fScaleFactor;

    WAIVESampler *plugin;

    FontId logo_font;

    Button *open_button;
    Waveform *waveform_display;

    DGL_NAMESPACE::FileBrowserOptions filebrowseropts;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WAIVESamplerUI);

};

UI *createUI()
{
    return new WAIVESamplerUI();
}


END_NAMESPACE_DISTRHO

#endif