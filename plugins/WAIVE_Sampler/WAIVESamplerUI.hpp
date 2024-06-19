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
#include "SourceBrowser.hpp"
#include "SourceBrowserRoot.hpp"
#include "HBox.hpp"
#include "VBox.hpp"
#include "Link.hpp"
#include "Menu.hpp"
#include "Knob.hpp"
#include "Knob3D.hpp"
#include "Label.hpp"
#include "Popup.hpp"
#include "Spinner.hpp"
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

#include <Poco/TaskNotification.h>
#include <Poco/Observer.h>
#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"
#include "Poco/Random.h"

START_NAMESPACE_DISTRHO

const unsigned int UI_W = 840;
const unsigned int UI_H = 582;

Knob3D *createWAIVEKnob();

class WAIVESamplerUI : public UI,
                       public Button::Callback,
                       public Waveform::Callback,
                       public Knob::Callback,
                       public SampleMap::Callback,
                       public TextInput::Callback,
                       public DropDown::Callback,
                       public SampleSlot::Callback,
                       public SourceBrowserRoot::Callback,
                       public SourceBrowser::Callback
{
public:
    WAIVESamplerUI();
    ~WAIVESamplerUI();

    void onTaskStarted(Poco::TaskStartedNotification *pNf);
    void onTaskFinished(Poco::TaskFinishedNotification *pNf);
    void onTaskFailed(Poco::TaskFailedNotification *pNf);
    void onTaskCancelled(Poco::TaskCancelledNotification *pNf);
    void onTaskProgress(Poco::TaskProgressNotification *pNf);
    void onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg);
    void onPluginUpdated(const void *pSender, const WAIVESampler::PluginUpdate &arg);

protected:
    void parameterChanged(uint32_t index, float value) override;
    void stateChanged(const char *key, const char *value) override;
    void onNanoDisplay() override;
    void uiScaleFactorChanged(const double scaleFactor) override;

    void buttonClicked(Button *button) override;
    void waveformSelection(Waveform *waveform, uint selectionStart) override;
    void knobDragStarted(Knob *knob) override;
    void knobDragFinished(Knob *knob, float value) override;
    void knobValueChanged(Knob *knob, float value) override;
    void mapSampleHovered(int id) override;
    void mapSampleSelected(int id) override;
    void mapSampleLoadSlot(int index, int slot) override;
    void textEntered(TextInput *textInput, std::string text) override;
    void textInputChanged(TextInput *textInput, std::string text) override;
    void dropdownSelection(DropDown *widget, int item) override;
    void sampleSelected(SampleSlot *slot) override;
    void sampleSlotCleared(SampleSlot *slot) override;
    void sourceBrowserClosed() override;
    void browserStopPreview() override;
    void browserLoadSource(const std::string &fp) override;

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
    Button *import_sample_btn, *open_source_btn, *new_sample_btn, *save_sample_btn, *play_btn,
        *expand_map_btn, *browser_sources_btn;
    Waveform *source_display, *sample_display;
    Knob3D *pitch, *volume, *percussionBoost;
    Knob3D *ampAttack, *ampDecay, *ampSustain, *ampRelease, *sustainLength;
    Knob3D *filterCutoff, *filterResonance;
    DropDown *filterType;
    SampleMap *sample_map;
    TextInput *sample_name;
    Spinner *import_spinner;

    Button *controls_toggle;
    Label *instructions;
    HBox *simple_buttons;
    Button *make_kick, *make_snare, *make_crash;

    bool map_full, simple_controls;

    std::vector<SampleSlot *> sampleSlots;
    std::vector<Button *> sampleTriggerButtons;
    std::vector<DropDown *> sampleMidiDropdowns;
    HBox *ampADSRKnobs, *shapeKnobs, *filterKnobs;
    VBox *slots_container;
    WidgetGroup *sample_editor_controls_advanced, *sample_editor_controls_simple;
    Menu *sample_map_menu, *dropdown_menu;
    Link *waive_link;

    SourceBrowserRoot *source_browser_root;
    SourceBrowser *source_browser;

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