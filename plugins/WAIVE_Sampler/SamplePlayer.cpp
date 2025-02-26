#include "SamplePlayer.hpp"

void SamplePlayer::addCallback(SamplePlayerCallback *cb)
{
    callbacks.push_back(cb);
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
    this->waveform = nullptr;
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