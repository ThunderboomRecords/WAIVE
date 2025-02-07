#ifndef WAIVE_SEQUENCER_UI_HPP
#define WAIVE_SEQUENCER_UI_HPP

#include "DistrhoUI.hpp"
#include "NanoVG.hpp"
#include "Window.hpp"

#include "Box.hpp"
#include "VBox.hpp"
#include "Knob.hpp"
#include "Icon.hpp"
#include "Label.hpp"
#include "Panel.hpp"
#include "TextInput.hpp"
#include "DropDown.hpp"
#include "Playhead.hpp"
#include "ScoreGrid.hpp"
#include "GrooveGraph.hpp"
#include "DrumPattern.hpp"
#include "SimpleButton.hpp"

#include "tinyfiledialogs.h"

#include "fonts.h"
#include "dropdown_icon.h"
#include "WAIVEColors.hpp"
#include "latent_distributions.h"

#include "WAIVESequencer.hpp"

START_NAMESPACE_DISTRHO

const unsigned int UI_W = 803;
const unsigned int UI_H = 579;

class WAIVESequencerUI : public UI,
                         public DrumPattern::Callback,
                         public GrooveGraph::Callback,
                         public Button::Callback,
                         public Knob::Callback,
                         public DropDown::Callback,
                         public TextInput::Callback
{
public:
    WAIVESequencerUI();
    ~WAIVESequencerUI();

protected:
    void parameterChanged(uint32_t index, float value) override;
    void stateChanged(const char *key, const char *value) override;
    bool onMotion(const MotionEvent &ev) override;
    void onNanoDisplay() override;
    void buttonClicked(Button *button) override;
    void grooveClicked(GrooveGraph *graph) override;
    void knobDragStarted(Knob *knob) override;
    void knobDragFinished(Knob *knob, float value) override;
    void knobValueChanged(Knob *knob, float value) override;
    void dropdownSelection(DropDown *widget, int item) override;
    void textEntered(TextInput *textInput, std::string text) override;
    void textInputChanged(TextInput *textInput, std::string text) override;
    void onDrumPatternClicked(DrumPattern *widget, int instrument, int sixteenth) override;
    void onDrumPatternScrolled(DrumPattern *widget, std::shared_ptr<Note> note, float deltaY) override;
    void onDrumPatternNoteMoved(DrumPattern *widget, std::shared_ptr<Note> note, uint32_t tick) override;
    void onNoteUpdated(DrumPattern *widget, std::shared_ptr<Note> note) override;
    void uiScaleFactorChanged(const double scaleFactor) override;

private:
    double fScaleFactor;

    WAIVESequencer *plugin;

    Panel *mainPanel;
    DrumPattern *drumPattern;
    GrooveGraph *grooveGraph;
    Label *scoreLabel, *grooveLabel, *complexityLabel, *midiLabel;
    std::vector<std::shared_ptr<Label>> drumNames;
    Button *newScoreBtn, *varScoreBtn, *newGrooveBtn, *varGrooveBtn, *quantizeBtn, *exportBtn;
    Knob *complexity;
    Box *scoreGenreBox, *grooveGenreBox;
    DropDown *scoreGenreDD, *grooveGenreDD;
    Icon *scoreDDIcon, *grooveDDIcon;
    std::vector<std::shared_ptr<TextInput>> midiNotesEdit;
    std::vector<std::shared_ptr<Knob>> complexities;

    Playhead *drumPlayhead;

    FontId fontTitle, fontMain;
    Label *toolTip;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WAIVESequencerUI);
};

UI *createUI()
{
    return new WAIVESequencerUI();
}

END_NAMESPACE_DISTRHO

#endif