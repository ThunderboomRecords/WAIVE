#ifndef SAMPLE_PLAYER_HPP
#define SAMPLE_PLAYER_HPP

#include <vector>

#include "SampleDatabase.hpp"

enum PlayState
{
    STOPPED = 0,
    TRIGGERED,
    PLAYING
};

struct SamplePlayer
{
    std::vector<float> *waveform = nullptr;
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
    void clear();
};

#endif