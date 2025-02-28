#ifndef SAMPLE_PLAYER_HPP
#define SAMPLE_PLAYER_HPP

#include <vector>
#include <mutex>

#include "SampleDatabase.hpp"

enum PlayState
{
    STOPPED = 0,
    TRIGGERED,
    PLAYING
};

class SamplePlayerCallback
{
public:
    virtual ~SamplePlayerCallback() {}
    virtual void sampleLoaded() = 0;
    virtual void sampleCleared() = 0;
};

struct SamplePlayer
{
    std::shared_ptr<std::vector<float>> waveform = nullptr;
    std::mutex waveformMtx;
    long length = 0;
    long ptr = 0;
    long startAt = 0;
    int midi = -1;
    float gain = 1.0f;
    float velocity = 0.8f;
    float pitch = 60.f;
    float pan = 0.0f;
    PlayState state = PlayState::STOPPED;
    bool active = false;
    std::shared_ptr<SampleInfo> sampleInfo = nullptr;
    void load(std::shared_ptr<SampleInfo> info);
    void loaded();
    void clear();
    void addCallback(std::shared_ptr<SamplePlayerCallback> cb);
    void removeCallback(std::shared_ptr<SamplePlayerCallback> cb);
    std::vector<std::shared_ptr<SamplePlayerCallback>> callbacks;
    SamplePlayer();
    SamplePlayer(const SamplePlayer &);
};

#endif