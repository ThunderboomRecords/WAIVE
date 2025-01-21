#ifndef WAIVE_SAMPLER_HPP
#define WAIVE_SAMPLER_HPP

#include <memory>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <queue>
#include <optional>
#include <mutex>

#include <fmt/core.h>
#include <sndfile.hh>

#include "DistrhoPluginInfo.h"
#include "DistrhoPlugin.hpp"
#include "WAIVESamplerParams.h"
#include "ThreadsafeQueue.hpp"
#include "SamplePlayer.hpp"
#include "SampleDatabase.hpp"
#include "Envelopes.hpp"
#include "WaveformFeatures.hpp"
#include "FeatureExtractor.hpp"
#include "Presets.hpp"

#include "samplerate.h"
#include "Gist.h"
#include "OSCClient.hpp"

#include <Poco/Task.h>
#include <Poco/TaskManager.h>
#include <Poco/TaskNotification.h>
#include <Poco/Observer.h>
#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"
#include "Poco/Exception.h"

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

class ImporterTask;
class FeatureExtractorTask;
class WaveformLoaderTask;

size_t loadWaveform(const char *fp, std::vector<float> &buffer, int sampleRate, int flags = 0);
bool saveWaveform(const char *fp, const float *buffer, sf_count_t size, int sampleRate);

class WAIVESampler : public Plugin
{
public:
    enum PluginUpdate
    {
        kSourceLoading = 0,
        kSourceLoaded,
        kSourceUpdated,
        kSampleLoading,
        kSampleLoaded,
        kSampleUpdated,
        kSampleCleared,
        kSampleAdded,
        kSlotLoaded,
        kParametersChanged,
    };

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

    void clear();
    void newSample();
    void loadSamplePreview(int id);
    void loadSourcePreview(const std::string &fp);
    void playSourcePreview();
    void stopSourcePreview();
    void loadSample(int id);
    void loadSample(std::shared_ptr<SampleInfo> s);
    void loadSource(const char *fp);
    void loadSlot(int slot, int id);
    void loadPreset(Preset preset);
    void selectWaveform(std::vector<float> *source, int start);
    void addCurrentSampleToLibrary();
    void generateCurrentSampleName(const std::string base);
    void renderSample();
    void loadSamplePlayer(std::shared_ptr<SampleInfo> info, SamplePlayer &sp, std::vector<float> &buffer);
    void clearSamplePlayer(SamplePlayer &sp);
    void triggerPreview();
    std::pair<float, float> getEmbedding(std::vector<float> *wf);
    void getFeatures(std::vector<float> *wf, std::vector<float> *feature);
    void onTaskFinished(Poco::TaskFinishedNotification *pNf);
    void onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg);

    const char *pluginUpdateToString(PluginUpdate update) const;

    EnvGen ampEnvGen;

private:
    Poco::TaskManager taskManager, converterManager;
    ImporterTask *importerTask;
    std::unique_ptr<WaveformLoaderTask> waveformLoaderTask;
    std::shared_ptr<std::vector<float>> tempBuffer;
    std::mutex tempBufferMutex;
    ThreadsafeQueue<std::string> import_queue;
    std::string fSourcePath;
    Poco::BasicEvent<const PluginUpdate> pluginUpdate;

    HTTPClient httpClient;

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

    SampleDatabase sd;
    OSCClient oscClient;

    std::shared_ptr<SampleInfo> fCurrentSample;

    std::vector<float> fSourceWaveform;
    bool fSourceLoaded, fSampleLoaded;
    int fSourceLength;
    std::string fSourceTagString;
    std::mutex sourceFeaturesMtx;
    std::vector<WaveformFeature> fSourceFeatures;
    std::vector<WaveformMeasurements> fSourceMeasurements;

    float fNormalisationRatio;
    Filter sampleFilter;

    std::mutex samplePlayerMtx;
    SamplePlayer *editorPreviewPlayer, *mapPreviewPlayer, *sourcePreviewPlayer;
    std::vector<float> *editorPreviewWaveform, *mapPreviewWaveform, *sourcePreviewWaveform;
    std::vector<SamplePlayer> samplePlayers;
    std::vector<std::vector<float>> samplePlayerWaveforms;

    friend class WAIVESamplerUI;
    friend class SampleEditorControls;
    friend class ImporterTask;
    friend class FeatureExtractorTask;
};

END_NAMESPACE_DISTRHO

#endif