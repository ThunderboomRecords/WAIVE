#ifndef WAIVE_MIDI_HPP
#define WAIVE_MIDI_HPP

#include <iostream>
#include <vector>
#include <set>
#include <assert.h>
#include <random>
#include <chrono>

#include "DistrhoPluginInfo.h"
#include "DistrhoPlugin.hpp"
#include "WAIVEMidiParams.h"
#include "Notes.hpp"

#include "model_utils.hpp"

#include "onnxruntime_cxx_api.h"
#include "score_decoder.onnx.h"
#include "score_encoder.onnx.h"
#include "groove_decoder.onnx.h"
#include "groove_encoder.onnx.h"
#include "full_groove_model.onnx.h"

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
        return d_version(0, 2, 1);
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
    void allNotesOff(uint32_t frame);

    void encodeGroove();
    void generateGroove();
    void encodeScore();
    void generateScore();
    void generateFullPattern();

    void computeNotes();

private:
    float fThreshold;

    double sampleRate;

    unsigned seed;
    std::default_random_engine generator;
    std::normal_distribution<float> distribution;
    
    Ort::SessionOptions sessionOptions;
    Ort::RunOptions mRunOptions {nullptr};
    Ort::Env mEnv {};

    // SCORE Model
    std::unique_ptr<Ort::Session> mScoreEncoder, mScoreDecoder;
    std::vector<std::vector<int64_t>> mScoreEncoderInputShape, mScoreEncoderOutputShape;
    std::vector<std::vector<int64_t>> mScoreDecoderInputShape, mScoreDecoderOutputShape;

    std::vector<std::string> mScoreEncoderInputNames, mScoreEncoderOutputNames;
    std::vector<std::string> mScoreDecoderInputNames, mScoreDecoderOutputNames;

    std::vector<float> mScoreZ;
    std::vector<float> mScoreInput, mScoreOutput;

    std::vector<Ort::Value> mScoreZTensor;
    std::vector<Ort::Value> mScoreInputTensor, mScoreOutputTensor;

    // GROOVE Model
    std::unique_ptr<Ort::Session> mGrooveEncoder, mGrooveDecoder;
    std::vector<std::vector<int64_t>> mGrooveEncoderInputShape, mGrooveEncoderOutputShape;
    std::vector<std::vector<int64_t>> mGrooveDecoderInputShape, mGrooveDecoderOutputShape;

    std::vector<std::string> mGrooveEncoderInputNames, mGrooveEncoderOutputNames;
    std::vector<std::string> mGrooveDecoderInputNames, mGrooveDecoderOutputNames;

    std::vector<float> mGrooveZ;
    std::vector<float> mGrooveInput, mGrooveOutput;

    std::vector<Ort::Value> mGrooveZTensor;
    std::vector<Ort::Value> mGrooveInputTensor, mGrooveOutputTensor;

    // FULL DECODER Model
    std::unique_ptr<Ort::Session> mFullDecoder;
    std::vector<std::vector<int64_t>> mFullDecoderInputShape, mFullDecoderOutputShape;
    std::vector<std::string> mFullDecoderInputNames, mFullDecoderOutputNames;

    std::vector<float> mFullZ, mFullOutput;
    std::vector<Ort::Value> mFullZTensor, mFullOutputTensor;




    std::vector<GrooveEvent> fGroove;
    float fScore[16][9];
    float fDrumPattern[16][30][3];

    const int max_events[9] = {3, 7, 3, 3, 3, 4, 3, 2, 2};
    int s_map[9];
    int ticks_per_beat;

    std::vector<Note> notes;
    std::vector<Note>::iterator notesPointer;
    std::set<uint8_t> triggered;

    float progress;
    double loopTick;

    friend class WAIVEMidiUI;
};


END_NAMESPACE_DISTRHO
#endif