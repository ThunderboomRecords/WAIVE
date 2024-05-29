#ifndef WAIVE_SAMPLER_UI_HPP
#define WAIVE_SAMPLER_UI_HPP

#include <queue>
#include <memory>
#include <thread>
#include <atomic>

#include "DistrhoUI.hpp"
#include "NanoVG.hpp"
#include "Window.hpp"

#include "WidgetGroup.hpp"
#include "HBox.hpp"
#include "VBox.hpp"
#include "Menu.hpp"
#include "Knob.hpp"
#include "Knob3D.hpp"
#include "Label.hpp"
#include "DropDown.hpp"
#include "Waveform.hpp"
#include "TextInput.hpp"
#include "SampleMap.hpp"
#include "SampleSlot.hpp"
#include "SimpleButton.hpp"
#include "ValueIndicator.hpp"

#include "Layout.hpp"

#include "fonts.h"

#include "WAIVESampler.hpp"
#include "WAIVESamplerParams.h"

#include "tinyfiledialogs.h"

START_NAMESPACE_DISTRHO

const unsigned int UI_W = 840;
const unsigned int UI_H = 582;

class WAIVESamplerUI : public UI,
                       public Button::Callback,
                       public IdleCallback,
                       public Waveform::Callback,
                       public Knob::Callback,
                       public SampleMap::Callback,
                       public TextInput::Callback,
                       public DropDown::Callback
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
    void waveformSelection(Waveform *waveform, uint selectionStart) override;
    void knobDragStarted(Knob *knob) override;
    void knobDragFinished(Knob *knob, float value) override;
    void knobValueChanged(Knob *knob, float value) override;
    void mapSampleHovered(int id) override;
    void mapSampleSelected(int id) override;
    void mapSampleLoadSlot(int index, int slot) override;
    void textEntered(TextInput *textInput, std::string text) override;
    void dropdownSelection(DropDown *widget, int item) override;

    void openFileBrowser(char *state, bool multiple);

private:
    void createSampleSlots();
    void setSampleEditorVisible(bool visible);

    float fScale;
    double fScaleFactor;

    WAIVESampler *plugin;

    FontId logo_font;

    std::thread open_dialog;
    std::atomic<bool> filebrowserOpen;

    Label *logo, *map_label, *sample_controls_label;
    ValueIndicator *value_indicator;
    Button *import_sample_btn, *open_source_btn, *new_sample_btn, *save_sample_btn, *play_btn;
    Waveform *source_display, *sample_display;
    Knob3D *pitch, *volume, *percussionBoost;
    Knob3D *ampAttack, *ampDecay, *ampSustain, *ampRelease, *sustainLength;
    Knob3D *filterCutoff, *filterResonance;
    DropDown *filterType;
    SampleMap *sample_map;
    TextInput *sample_name;

    std::vector<SampleSlot *> sampleSlots;
    std::vector<Button *> sampleTriggerButtons;
    std::vector<DropDown *> sampleMidiDropdowns;
    HBox *ampADSRKnobs, *shapeKnobs, *filterKnobs;
    VBox *slots_container;
    WidgetGroup *sample_editor_controls;
    Menu *sample_map_menu, *dropdown_menu;

    DGL_NAMESPACE::FileBrowserOptions filebrowseropts;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WAIVESamplerUI);
};

Knob3D *createWAIVEKnob(
    WAIVESamplerUI *parent,
    Parameters param,
    std::string label,
    float min,
    float max,
    float value,
    UI::FontId font);

UI *createUI()
{
    return new WAIVESamplerUI();
}

END_NAMESPACE_DISTRHO

#endif