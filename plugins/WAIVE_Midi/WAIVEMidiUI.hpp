#ifndef WAIVE_MIDIUI_HPP
#define WAIVE_MIDIUI_HPP

#include "DistrhoUI.hpp"
#include "NanoVG.hpp"
#include "Window.hpp"

#include "VBox.hpp"
#include "Knob.hpp"
#include "Label.hpp"
#include "Panel.hpp"
#include "DropDown.hpp"
#include "Playhead.hpp"
#include "ScoreGrid.hpp"
#include "GrooveGraph.hpp"
#include "DrumPattern.hpp"
#include "SimpleButton.hpp"

#include "fonts.h"
#include "WAIVEColors.hpp"

#include "WAIVEMidi.hpp"

START_NAMESPACE_DISTRHO

const unsigned int UI_W = 880;
const unsigned int UI_H = 460;

class WAIVEMidiUI : public UI,
                    public GrooveGraph::Callback,
                    public Button::Callback,
                    public Knob::Callback,
                    public DropDown::Callback
{
public:
    WAIVEMidiUI();
    ~WAIVEMidiUI();

protected:
    void parameterChanged(uint32_t index, float value) override;
    void stateChanged(const char *key, const char *value) override;
    void onNanoDisplay() override;
    void buttonClicked(Button *button) override;
    void grooveClicked(GrooveGraph *graph) override;
    void knobDragStarted(Knob *knob) override;
    void knobDragFinished(Knob *knob, float value) override;
    void knobValueChanged(Knob *knob, float value) override;
    void dropdownSelection(DropDown *widget, int item) override;
    void uiScaleFactorChanged(const double scaleFactor) override;

private:
    double fScaleFactor;

    WAIVEMidi *plugin;

    Panel *edit_panel, *result_panel;
    ScoreGrid *score_grid;
    GrooveGraph *groove_graph;
    DrumPattern *drum_pattern;
    Label *score_label, *groove_label, *threshold_label;
    std::vector<std::shared_ptr<Label>> drum_names;
    Button *new_score, *var_score, *new_groove, *var_groove;
    Knob *threshold;
    std::vector<std::shared_ptr<DropDown>> midi_notes;

    Playhead *drum_playhead;

    FontId logo_font;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WAIVEMidiUI);
};

UI *createUI()
{
    return new WAIVEMidiUI();
}

END_NAMESPACE_DISTRHO

#endif