#include "SamplePlayer.hpp"

SamplePlayer::SamplePlayer() : length(0),
                               ptr(0),
                               midi(-1),
                               gain(1.0f),
                               velocity(0.8f),
                               pitch(60.f),
                               pan(0.0f),
                               state(PlayState::STOPPED),
                               active(false),
                               sampleInfo(nullptr),
                               waveformMtx(std::make_shared<std::mutex>())
{
}

SamplePlayer::SamplePlayer(const SamplePlayer &other)
{
    waveform = other.waveform;
    waveformMtx = std::make_shared<std::mutex>();
    length = other.length;
    ptr = other.ptr;
    midi = other.midi;
    gain = other.gain;
    velocity = other.velocity;
    pitch = other.pitch;
    pan = other.pan;
    state = other.state;
    active = other.active;
    sampleInfo = other.sampleInfo;
}

void SamplePlayer::addCallback(SamplePlayerCallback *cb)
{
    callbacks.push_back(cb);
}

void SamplePlayer::removeCallback(SamplePlayerCallback *cb)
{
    callbacks.erase(std::remove(callbacks.begin(), callbacks.end(), cb), callbacks.end());
}

void SamplePlayer::load(std::shared_ptr<SampleInfo> info)
{
    if (info == nullptr)
    {
        clear();
        return;
    }

    state = PlayState::STOPPED;
    ptr = 0;
    active = false;
    sampleInfo = info;
}

void SamplePlayer::loaded()
{
    for (auto cb : callbacks)
        cb->sampleLoaded();
}

void SamplePlayer::clear()
{
    this->waveform->clear();
    this->length = 0;
    this->ptr = 0;
    this->midi = -1;
    this->gain = 1.0f;
    this->velocity = 0.8f;
    this->pitch = 60.f;
    this->pan = 0.0f;
    this->state = PlayState::STOPPED;
    this->active = false;
    this->sampleInfo = nullptr;

    for (auto cb : callbacks)
        cb->sampleCleared();
}