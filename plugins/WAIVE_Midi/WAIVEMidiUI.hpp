#ifndef WAIVE_MIDIUI_HPP
#define WAIVE_MIDIUI_HPP

#include "DistrhoUI.hpp"
#include "NanoVG.hpp"
#include "Window.hpp"

#include "HBox.hpp"
#include "VBox.hpp"
#include "VSlider.hpp"
#include "ScoreGrid.hpp"
#include "GrooveGraph.hpp"
#include "DrumPattern.hpp"

#include "WAIVEMidi.hpp"


START_NAMESPACE_DISTRHO


const unsigned int UI_W = 800;
const unsigned int UI_H = 380;


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
    HBox *hbox_controls;
    VBox *vbox_container;
    ScoreGrid *score_grid;
    GrooveGraph *groove_graph;
    DrumPattern *drum_pattern;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WAIVEMidiUI);
};

UI *createUI()
{
    return new WAIVEMidiUI();
}

END_NAMESPACE_DISTRHO

#endif