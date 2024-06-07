#ifndef WAIVE_SAMPLER_HPP
#define WAIVE_SAMPLER_HPP

#include <memory>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <queue>

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
#include "SimpleUDP.hpp"
#include "FeatureExtractor.hpp"

#include "samplerate.h"
#include "Gist.h"
#include <tinyosc.h>

#include "model_utils.hpp"
#include "onnxruntime_cxx_api.h"
#include "tsne.c"

#include "version_config.h"

#ifdef WAIVE_PLUGINS_VERSION_INFO
const int V_MAJ = WAIVE_PLUGINS_VERSION_MAJOR;
const int V_MIN = WAIVE_PLUGINS_VERSION_MINOR;
const int V_PAT = WAIVE_PLUGINS_VERSION_PATCH;
#else
const int V_MAJ = 1;
const int V_MIN = 0;
const int V_PAT = 0;
#endif

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
        return d_version(V_MAJ, V_MIN, V_PAT);
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
    void importSample(const char *fp);
    void loadPreview(int id);
    void loadSample(int id);
    void loadSample(std::shared_ptr<SampleInfo> s);
    void loadSource(const char *fp);
    void loadSlot(int slot, int id);
    int loadWaveform(const char *fp, std::vector<float> *buffer);
    bool saveWaveform(const char *fp, float *buffer, sf_count_t size);
    void selectWaveform(std::vector<float> *source, int start);
    void addCurrentSampleToLibrary();
    void renderSample();
    void loadSamplePlayer(std::shared_ptr<SampleInfo> info, SamplePlayer &sp, std::vector<float> &buffer);
    void triggerPreview();
    std::pair<float, float> getEmbedding(std::vector<float> *wf);
    void getFeatures(std::vector<float> *wf, std::vector<float> *feature);
    void getOnsets();

    EnvGen ampEnvGen;

private:
    void addToUpdateQueue(int ev);

    float sampleRate;
    FeatureExtractor fe;

    Ort::SessionOptions sessionOptions;
    Ort::RunOptions mRunOptions{nullptr};
    Ort::Env mEnv{};

    std::unique_ptr<Ort::Session> mTSNE;
    std::vector<std::string> mTSNEInputNames, mTSNEOutputNames;
    std::vector<std::vector<int64_t>> mTSNEInputShape, mTSNEOutputShape;
    std::vector<float> mTSNEInput, mTSNEOutput;
    std::vector<Ort::Value> mTSNEInputTensor, mTSNEOutputTensor;

    Gist<float> gist;

    SampleDatabase sd;
    SimpleUDPServer server;
    char oscBuffer[2048];

    std::shared_ptr<SampleInfo> fCurrentSample;

    std::vector<float> fSourceWaveform;
    bool fSourceLoaded, fSampleLoaded;
    int fSourceLength;
    std::vector<WaveformFeature> fSourceFeatures;

    float fNormalisationRatio;
    Filter sampleFilter;

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