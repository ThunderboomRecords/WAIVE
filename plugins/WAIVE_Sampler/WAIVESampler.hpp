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
#include "SampleDatabase.hpp"
#include "Envelopes.hpp"

#include <librosa.h>
#include "samplerate.h"

#define MAX_PATH 128

namespace fs = std::filesystem;

fs::path get_homedir();

START_NAMESPACE_DISTRHO

enum PlayState
{
    STOPPED = 0,
    TRIGGERED,
    PLAYING
};

struct SamplePlayer
{
    std::vector<float> *waveform;
    int length;
    int ptr = 0;
    PlayState state = PlayState::STOPPED;
};

class WAIVESampler : public Plugin
{
public:
    WAIVESampler();
    ~WAIVESampler();

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
    void run(const float **, float **outputs, uint32_t numFrames, const MidiEvent *midiEvents, uint32_t midiEventCount) override;
    void sampleRateChanged(double newSampleRate) override;

    void newSample();
    void loadSample(int id);
    void loadSample(SampleInfo *s);
    void loadSource(const char *fp);
    bool loadWaveform(const char *fp, std::vector<float> *buffer);
    bool saveWaveform(const char *fp, float *buffer, sf_count_t size);
    void selectWaveform(std::vector<float> *source, uint start, bool process);
    void addToLibrary();
    void renderSample();
    void getEmbeding();
    void analyseWaveform();

private:
    void addToUpdateQueue(int ev);

    float sampleRate;

    fs::path fCacheDir;
    SampleDatabase *db;
    std::vector<SampleInfo> fAllSamples;

    SampleInfo *fCurrentSample;

    float fSampleVolume;
    float fSamplePitch;
    ADSR_Params fAmpADSRParams;
    float fSustainLength;
    EnvGen ampEnvGen;

    std::vector<float> fSourceWaveform;
    std::string fSourceFilepath;
    bool fSourceLoaded;
    int fSourceLength;

    std::vector<float> fSamplePitched, fSample;
    bool fSampleLoaded;
    int fSampleStart, fSampleLength;
    float fNormalisationRatio;

    SamplePlayer previewPlayer;

    std::queue<int> updateQueue;

    std::thread *tWaveShaping;

    friend class WAIVESamplerUI;
};

END_NAMESPACE_DISTRHO

#endif