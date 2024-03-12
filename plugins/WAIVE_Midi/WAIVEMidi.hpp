#ifndef WAIVE_MIDI_HPP
#define WAIVE_MIDI_HPP


#include "DistrhoPluginInfo.h"
#include "DistrhoPlugin.hpp"
#include "WAIVEMidiParams.h"

#include <torch/script.h>
#include "score_decoder.h"
#include "groove_decoder.h"
#include "full_groove_model.h"
#include "latent_distributions.h"

START_NAMESPACE_DISTRHO

class WAIVEMidi : public Plugin
{
public:
    WAIVEMidi();

protected:
    const char *getLabel() const noexcept override
    {
        return "WAIVE Midi";
    }

    const char *getDescription() const override
    {
        return "WAIVE Midi Generator";
    }

    const char *getMaker() const noexcept override
    {
        return "Thunderboom Records";
    }

    const char *getHomePage() const override
    {
        return "https://github.com/ThunderboomRecords/WAIVE";
    }

    const char *getLicense() const noexcept override
    {
        return "GPL V3";
    }

    uint32_t getVersion() const noexcept override
    {
        return d_version(1, 0, 0);
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('t', 'b', 'W', 'M');
    }

    void initParameter(uint32_t index, Parameter &parameter) override;
    void setState(const char *key, const char *value) override;
    String getState(const char *key) const override;
    void initState(unsigned int, String &, String &) override;

    // --- Internal data ----------
    float getParameterValue(uint32_t index) const override;
    void setParameterValue(uint32_t index, float value) override;

    // --- Process ----------------
    // void activate() override;
    void run(const float **, float **, uint32_t numFrames, const MidiEvent *midiEvents, uint32_t midiEventCount) override;
    void sampleRateChanged(double newSampleRate) override;

    void generateGroove();
    void generateScore();
    void generateFullPattern();

private:
    float fThreshold;

    torch::jit::script::Module score_decoder_model, groove_decoder_model, full_model;

    torch::Tensor pattern;
    torch::Tensor score_z, groove_z;

    torch::Tensor score_m, score_s, groove_m, groove_s; 

    float fGroove[48][3];
    float fScore[16][9];

    friend class WAIVEMidiUI;
};


END_NAMESPACE_DISTRHO
#endif