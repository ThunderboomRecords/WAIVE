#ifndef WAIVE_SAMPLER_HPP
#define WAIVE_SAMPLER_HPP

#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <queue>

#include <fstream>
#include <mutex>
#include <stdlib.h>
#include <time.h>
#include <algorithm>

#include <fmt/core.h>
#include <sndfile.hh>

#include "DistrhoPluginInfo.h"
#include "DistrhoPlugin.hpp"
#include "WAIVESamplerParams.h"
#include "SampleDatabase.hpp"
#include "Envelopes.hpp"
#include "WaveformFeatures.hpp"

#include <librosa.h>
#include "samplerate.h"
#include "Gist.h"

#define MAX_PATH 128

START_NAMESPACE_DISTRHO

enum PlayState
{
    STOPPED = 0,
    TRIGGERED,
    PLAYING
};

struct SamplePlayer
{
    std::vector<float> *waveform = nullptr;
    int length = 0;
    int ptr = 0;
    int midi = -1;
    float gain = 1.0f;
    float velocity = 0.8f;
    float pitch = 60.f;
    float pan = 0.0f;
    PlayState state = PlayState::STOPPED;
    bool active = false;
    std::shared_ptr<SampleInfo> sampleInfo = nullptr;
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
    // std::shared_ptr<SampleInfo> findSample(int id);
    void loadPreview(int id);
    void loadSample(int id);
    void loadSample(std::shared_ptr<SampleInfo> s);
    void loadSource(const char *fp);
    void loadSlot(int slot, int id);
    int loadWaveform(const char *fp, std::vector<float> *buffer);
    bool saveWaveform(const char *fp, float *buffer, sf_count_t size);
    void selectWaveform(std::vector<float> *source, int start);
    void addCurrentSampleToLibrary();
    // bool renameCurrentSample(std::string new_name);
    // bool saveSamples();
    void renderSample();
    void loadSamplePlayer(std::shared_ptr<SampleInfo> info, SamplePlayer &sp, std::vector<float> &buffer);
    void triggerPreview();
    void getEmbedding();
    void analyseWaveform();
    void getOnsets();

    EnvGen ampEnvGen;

private:
    void addToUpdateQueue(int ev);

    float sampleRate;

    Gist<float> gist;

    SampleDatabase sd;

    // fs::path fCacheDir;

    std::shared_ptr<SampleInfo> fCurrentSample;

    std::vector<float> fSourceWaveform;
    bool fSourceLoaded, fSampleLoaded;
    int fSourceLength;
    std::vector<WaveformFeature> fSourceFeatures;

    float fNormalisationRatio;

    std::mutex samplePlayerMtx;
    SamplePlayer *editorPreviewPlayer, *mapPreviewPlayer;
    std::vector<float> *editorPreviewWaveform, *mapPreviewWaveform;
    std::vector<SamplePlayer> samplePlayers;
    std::vector<std::vector<float>> samplePlayerWaveforms;

    std::queue<int> updateQueue;

    friend class WAIVESamplerUI;
    friend class SampleEditorControls;
};

END_NAMESPACE_DISTRHO

#endif