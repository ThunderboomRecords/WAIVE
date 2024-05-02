#ifndef WAIVE_SAMPLER_HPP
#define WAIVE_SAMPLER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <filesystem>
#include <fstream>
#include <thread>
#include <stdlib.h>
#include <time.h>

#include <fmt/core.h>
#include <sndfile.hh>

#include "DistrhoPluginInfo.h"
#include "DistrhoPlugin.hpp"
#include "WAIVESamplerParams.h"

#include <librosa.h>
#include "signalsmith-stretch.h"
#include "csv.h"

#define MAX_PATH 128

namespace fs = std::filesystem;

fs::path get_homedir();

START_NAMESPACE_DISTRHO

class WAIVESampler : public Plugin
{
public:
    WAIVESampler();

protected:
    const char *getLabel() const noexcept override
    {
        return "WAIVE Sampler";
    }

    const char *getDescription() const override
    {
        return "WAIVE Sampler";
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
        return d_version(0, 3, 0);
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('t', 'b', 'W', 'S');
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
    void run(const float **, float **outputs, uint32_t numFrames, const MidiEvent *midiEvents, uint32_t midiEventCount) override;
    void sampleRateChanged(double newSampleRate) override;

    bool loadWaveform(const char *fp, std::vector<float> *buffer);
    bool saveWaveform(const char *fp, float *buffer, sf_count_t size);
    void selectSample(std::vector<float> *source, uint start, uint end);
    void addToLibrary();
    void repitchSample();
    void renderSample();
    void analyseWaveform();

private:
    void addToUpdateQueue(int ev);

    float sampleRate;

    fs::path fCacheDir;

    signalsmith::stretch::SignalsmithStretch<float> stretch;

    float fSampleVolume;
    float fSamplePitch;
    std::string fFilepath;

    std::vector<float> fSourceWaveform;
    bool fSourceLoaded;

    std::vector<float> fSampleRaw, fSamplePitched, fSample;
    bool fSampleLoaded;
    uint fSampleLength, fSamplePtr;

    std::queue<int> updateQueue;

    std::thread *tWaveShaping;

    friend class WAIVESamplerUI;
};

END_NAMESPACE_DISTRHO

#endif