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
};

enum PresetName
{
    KICK = 0,
    SNARE,
    CRASH
};

namespace Presets
{
    static Preset KickPreset = {
        1.5f,
        0.1f,
        0.12f,
        0.2f,
        0.0f,
        Filter::FILTER_LOWPASS,
        {0.0f,
         20.f,
         0.2f,
         50.f},
        50.f};

    static Preset SnarePreset = {
        1.0f,
        1.0f,
        0.0f,
        0.1f,
        0.2f,
        Filter::FILTER_BANDPASS,
        {0.0f,
         60.f,
         0.6f,
         100.f},
        50.f};

    static Preset HiHat = {
        1.0f,
        1.0f,
        2.0f,
        0.2f,
        0.3f,
        Filter::FILTER_HIGHPASS,
        {2.0f,
         50.f,
         0.6f,
         120.f},
        50.f};

    static Preset Clap = {
        1.0f,
        1.0f,
        1.5f,
        0.2f,
        0.3f,
        Filter::FILTER_BANDPASS,
        {2.0f,
         50.f,
         0.6f,
         120.f},
        50.f};
} // namespace Presets

#endif