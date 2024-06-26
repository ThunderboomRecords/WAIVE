#ifndef WAIVE_MIDIUI_HPP
#define WAIVE_MIDIUI_HPP

#include "DistrhoUI.hpp"
#include "NanoVG.hpp"
#include "Window.hpp"

#include "HBox.hpp"
#include "VBox.hpp"
#include "VSlider.hpp"
#include "Playhead.hpp"
#include "ScoreGrid.hpp"
#include "GrooveGraph.hpp"
#include "DrumPattern.hpp"

#include "fonts.h"
#include "WAIVEColors.hpp"

#include "WAIVEMidi.hpp"

START_NAMESPACE_DISTRHO

const unsigned int UI_W = 840;
const unsigned int UI_H = 380;

class WAIVEMidiUI : public UI,
                    public GrooveGraph::Callback
{
public:
    WAIVEMidiUI();
    ~WAIVEMidiUI();

protected:
    void parameterChanged(uint32_t index, float value) override;
    void stateChanged(const char *key, const char *value) override;
    void onNanoDisplay() override;
    void grooveClicked(GrooveGraph *graph) override;
    void uiScaleFactorChanged(const double scaleFactor) override;

private:
    float fScale;
    double fScaleFactor;

    WAIVEMidi *plugin;
    VSlider *fThreshold;
    HBox *hbox_controls;
    VBox *vbox_container;
    ScoreGrid *score_grid;
    GrooveGraph *groove_graph;
    DrumPattern *drum_pattern;

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