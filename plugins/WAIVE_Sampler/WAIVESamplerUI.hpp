#ifndef WAIVE_SAMPLER_UI_HPP
#define WAIVE_SAMPLER_UI_HPP

#include <queue>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "DistrhoUI.hpp"
#include "NanoVG.hpp"
#include "Window.hpp"

#include "WidgetGroup.hpp"
#include "SampleBrowser.hpp"
#include "SampleBrowserRoot.hpp"
#include "Box.hpp"
#include "HBox.hpp"
#include "VBox.hpp"
#include "Link.hpp"
#include "Menu.hpp"
#include "Icon.hpp"
#include "Knob.hpp"
#include "Knob3D.hpp"
#include "Panel.hpp"
#include "Label.hpp"
#include "Image.hpp"
#include "TagBrowser.hpp"
#include "Spinner.hpp"
#include "DropDown.hpp"
#include "RadioButtons.hpp"
#include "Waveform.hpp"
#include "TextInput.hpp"
#include "SampleMap.hpp"
#include "SampleSlot.hpp"
#include "SourceList.hpp"
#include "SimpleButton.hpp"
#include "ValueIndicator.hpp"
#include "DragDrop.hpp"

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

#include "WAIVEUtils.hpp"

START_NAMESPACE_DISTRHO

const unsigned int UI_W = 862;
const unsigned int UI_H = 662;

class WAIVESamplerUI;

class WAIVESamplerUI : public UI,
                       public Button::Callback,
                       public Waveform::Callback,
                       public Knob::Callback,
                       public SampleMap::Callback,
                       public TextInput::Callback,
                       public Menu::Callback,
                       public DropDown::Callback,
                       public SampleSlot::Callback,
                       public SourceList::Callback,
                       public DragDropManager::Callback
{
public:
    WAIVESamplerUI();
    ~WAIVESamplerUI();

    void updateWidgets();

    void onTaskStarted(Poco::TaskStartedNotification *pNf);
    void onTaskFinished(Poco::TaskFinishedNotification *pNf);
    void onTaskFailed(Poco::TaskFailedNotification *pNf);
    void onTaskCancelled(Poco::TaskCancelledNotification *pNf);
    void onTaskProgress(Poco::TaskProgressNotification *pNf);
    void onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg);
    void onPluginUpdated(const void *pSender, const WAIVESampler::PluginUpdate &arg);
    void onSlotLoaded(const void *pSender, int &slotId);
    void onSlotUnloaded(const void *pSender, int &slotId);

protected:
    // Plugin callbacks
    void parameterChanged(uint32_t index, float value) override;
    void stateChanged(const char *key, const char *value) override;
    void onNanoDisplay() override;
    void uiScaleFactorChanged(const double scaleFactor) override;

    // Interaction callbacks
    bool onMotion(const MotionEvent &ev) override;
    bool onMouse(const MouseEvent &ev) override;

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
    void mapSampleDelete(int id) override;
    void textEntered(TextInput *textInput, const std::string &text) override;
    void textInputChanged(TextInput *textInput, const std::string &text) override;
    void dropdownSelection(DropDown *widget, int item) override;
    void onMenuItemSelection(Menu *menu, int item, const std::string &value) override;
    void sampleSelected(SampleSlot *slot, int slotId) override;
    void sampleSlotCleared(SampleSlot *slot, int slotId) override;
    void sampleSlotLoadSample(SampleSlot *slot, int slotId, int sampleId) override;

    // Source List Callbacks
    void sourceDownload(int index) override;
    void sourcePreview(int index, bool start) override;
    void sourceLoad(int index) override;

    // DragDrop Callbacks
    void onDragDropStart(DragDropWidget *widget, DragDropEvent &ev) override;
    void onDragDropEnd(DragDropWidget *widget, DragDropEvent ev, bool accepted) override;

private:
    void beginOpenFileBrowser(const std::string &state, bool multiple);
    void updatePresetButtons();

    Knob *createWAIVEKnob(
        Parameters param,
        const std::string &label,
        float min,
        float max,
        float value);

    float fScale;
    double fScaleFactor;
    int lastRequestedDownload;

    FontId fontTitle, fontMain;

    WAIVESampler *plugin;

    Poco::Random random;

    std::thread open_dialog;
    std::atomic<bool> filebrowserOpen;
    std::mutex fileBrowserOpenMtx;
    std::condition_variable fileBrowserCV;

    bool errorMessage;
    int loadingTaskCount;

    Panel *sourceBrowserPanel, *sampleEditorPanel, *samplePlayerPanel;

    // 1. Source Browser Components
    SourceList *sourceList;
    Spinner *databaseLoading;
    Button *openFilterPanelBtn, *previewPlaybackBtn, *importSource, *randomSourceBtn;
    Box *searchBox;
    TextInput *sourceSearch;
    RadioButtons *archiveList;
    Icon *searchIcon;
    Label *databaseProgress, *archiveListHeading, *browseTagsHeading;

    TagBrowser *tagBrowser;

    // 2. Sample Editor Components
    HBox *presetButtons, *editorKnobs;
    Button *makeKick, *makeSnare, *makeHihat, *makeClap;
    Label *presetLabel, *knobsLabel;
    Knob *pitch, *volume, *percussionBoost;
    Knob *ampAttack, *ampDecay, *ampSustain, *ampRelease, *sustainLength;
    Knob *filterCutoff, *filterResonance;
    DropDown *filterType;
    Spinner *sourceLoading;
    Label *instructions, *progress;

    Waveform *sampleWaveformDisplay;
    Button *saveSampleBtn, *playSampleBtn;

    // 3. Sample Player Components
    VBox *sampleSlotsContainer;
    std::vector<SampleSlot *> sampleSlots;
    Button *openMapBtn, *browseFilesBtn, *oscControlsBtn, *oscEnableBtn;
    Panel *oscControlsPanel;
    TextInput *oscHostInput, *oscPortInput;
    Label *oscControlsDescription;

    SampleBrowser *sampleBrowser;

    Label *toolTip, *dragInfo;
    DragDropManager *dragDropManager;
    std::vector<Knob *> allKnobs;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WAIVESamplerUI);
};

UI *createUI()
{
    return new WAIVESamplerUI();
}

END_NAMESPACE_DISTRHO

#endif