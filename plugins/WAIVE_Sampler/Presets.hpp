#ifndef PRESETS_HPP_INCLUDED
#define PRESETS_HPP_INCLUDED

#include "Filters.hpp"
#include "Envelopes.hpp"

struct Preset
{
    float volume;
    float pitch;
    float percussiveBoost;
    float filterCutoff;
    float filterResonance;
    Filter::FilterType filterType;
    ADSR_Params adsr;
    float sustainLength;
    std::string presetName;
};

namespace Presets
{
    static Preset Kick = {
        .volume = 1.0f,
        .pitch = 0.1f,
        .percussiveBoost = 0.12f,
        .filterCutoff = 0.2f,
        .filterResonance = 0.0f,
        .filterType = Filter::FILTER_LOWPASS,
        .adsr = {.attack = 0.0f,
                 .decay = 20.f,
                 .sustain = 0.2f,
                 .release = 50.f},
        .sustainLength = 50.f,
        .presetName = "kick"};

    static Preset Snare = {
        .volume = 1.0f,
        .pitch = 1.0f,
        .percussiveBoost = 0.0f,
        .filterCutoff = 0.1f,
        .filterResonance = 0.2f,
        .filterType = Filter::FILTER_BANDPASS,
        .adsr = {.attack = 0.0f,
                 .decay = 60.f,
                 .sustain = 0.6f,
                 .release = 100.f},
        .sustainLength = 50.f,
        .presetName = "snare"};

    static Preset HiHat = {
        .volume = 1.0f,
        .pitch = 1.0f,
        .percussiveBoost = 2.0f,
        .filterCutoff = 0.2f,
        .filterResonance = 0.3f,
        .filterType = Filter::FILTER_HIGHPASS,
        .adsr = {.attack = 2.0f,
                 .decay = 50.f,
                 .sustain = 0.6f,
                 .release = 120.f},
        .sustainLength = 50.f,
        .presetName = "hihat"};

    static Preset Clap = {
        .volume = 1.0f,
        .pitch = 1.0f,
        .percussiveBoost = 1.5f,
        .filterCutoff = 0.2f,
        .filterResonance = 0.3f,
        .filterType = Filter::FILTER_BANDPASS,
        .adsr = {.attack = 2.0f,
                 .decay = 50.f,
                 .sustain = 0.6f,
                 .release = 120.f},
        .sustainLength = 50.f,
        .presetName = "clap"};

    static Preset Any = {
        .volume = 1.0f,
        .pitch = 1.0f,
        .percussiveBoost = 0.1f,
        .filterCutoff = 0.3f,
        .filterResonance = 0.1f,
        .filterType = Filter::FILTER_BANDPASS,
        .adsr = {.attack = 2.0f,
                 .decay = 50.f,
                 .sustain = 0.8f,
                 .release = 200.f},
        .sustainLength = 150.f,
        .presetName = "hit"};
} // namespace Presets

#endif