#ifndef WAVEFORM_FEATURES_HPP_INCLUDED
#define WAVEFORM_FEATURES_HPP_INCLUDED

#include "FeatureTypes.h"
#include <string>

struct WaveformFeature
{
    FeatureType type;
    std::string label;
    float value;
    int start;
    int end;
};

#endif