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
#include "SampleBrowser.hpp"
#include "SampleBrowserRoot.hpp"
#include "HBox.hpp"
#include "VBox.hpp"
#include "Link.hpp"
#include "Menu.hpp"
#include "Icon.hpp"
#include "Knob.hpp"
#include "Knob3D.hpp"
#include "Label.hpp"
#include "Popup.hpp"
#include "Image.hpp"
#include "Spinner.hpp"
#include "DropDown.hpp"
#include "Waveform.hpp"
#include "TextInput.hpp"
#include "SampleMap.hpp"
#include "SampleSlot.hpp"
#include "SourceList.hpp"
#include "SimpleButton.hpp"
#include "ValueIndicator.hpp"

#include "Layout.hpp"

#include "fonts.h"
#include "icons/search.h"

#include "WAIVESampler.hpp"
#include "WAIVESamplerParams.h"

#include "tinyfiledialogs.h"

#include <Poco/TaskNotification.h>
#include <Poco/Observer.h>
#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"
#include "Poco/Random.h"

START_NAMESPACE_DISTRHO

const unsigned int UI_W = 1000;
const unsigned int UI_H = 582;

class WAIVESamplerUI;

Knob *createWAIVEKnob(
    WAIVESamplerUI *parent,
    Parameters param,
    std::string label,
    float min,
    float max,
    float value,
    UI::FontId font);

class WAIVESamplerUI : public UI,
                       public Button::Callback,
                       public Waveform::Callback,
                       public Knob::Callback,
                       public SampleMap::Callback,
                       public TextInput::Callback,
                       public DropDown::Callback,
                       public SampleSlot::Callback,
                       public SampleBrowserRoot::Callback,
                       public SourceList::Callback
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
    // Plugin callbacks
    void parameterChanged(uint32_t index, float value) override;
    void stateChanged(const char *key, const char *value) override;
    void onNanoDisplay() override;
    void uiScaleFactorChanged(const double scaleFactor) override;

    // Widget Callbacks
    void buttonClicked(Button *button) override;
    void waveformSelection(Waveform *waveform, uint selectionStart) override;
    void knobDragStarted(Knob *knob) override;
    void knobDragFinished(Knob *knob, float value) override;
    void knobValueChanged(Knob *knob, float value) override;
    void mapSampleHovered(int id) override;
    void mapSampleSelected(int id) override;
    void mapSampleLoadSlot(int index, int slot) override;
    void mapSampleImport() override;
    void textEntered(TextInput *textInput, std::string text) override;
    void textInputChanged(TextInput *textInput, std::string text) override;
    void dropdownSelection(DropDown *widget, int item) override;
    void sampleSelected(SampleSlot *slot, int slotId) override;
    void sampleSlotCleared(SampleSlot *slot, int slotId) override;

    // Source List Callbacks
    void sourceDownload(int index) override;
    void sourcePreview(int index) override;
    void sourceLoad(int index) override;

private:
    void beginOpenFileBrowser(const std::string &state, bool multiple);

    float fScale;
    double fScaleFactor;

    WAIVESampler *plugin;

    Poco::Random random;

    FontId logo_font;

    std::thread open_dialog;
    std::atomic<bool> filebrowserOpen;

    Panel *sourceBrowserPanel, *sampleEditorPanel, *samplePanel, *samplePlayerPanel;

    // 1. Source Browser Components
    SourceList *sourceList;
    Spinner *loading;
    Button *filterSources;
    Panel *searchBox;
    TextInput *sourceSearch;
    Icon *searchIcon;

    // 2. Sample Editor Components
    Waveform *sourceWaveformDisplay;
    HBox *presetButtons, *editorKnobs;
    Button *makeKick, *makeSnare, *makeHihat, *makeClap, *importSource;
    Label *presetLabel, *knobsLabel;
    Knob *pitch, *volume, *percussionBoost;
    Knob *ampAttack, *ampDecay, *ampSustain, *ampRelease, *sustainLength;
    Knob *filterCutoff, *filterResonance;
    DropDown *filterType;
    Spinner *sourceLoading;
    Label *instructions, *progress;

    // 3. Sample Panel Components
    Waveform *sampleWaveformDisplay;
    TextInput *sampleName;
    Button *saveSampleBtn, *playSampleBtn; // *newSampleBtn;

    // 4. Sample Pack Components
    VBox *sampleSlotsContainer;
    std::vector<SampleSlot *> sampleSlots;
    Button *openMapBtn;

    SampleBrowserRoot *sampleBrowserRoot;
    SampleBrowser *sampleBrowser;

    // OTHER
    ValueIndicator *valueIndicator;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WAIVESamplerUI);
};

UI *createUI()
{
    return new WAIVESamplerUI();
}

END_NAMESPACE_DISTRHO

#endif