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
    int index;
};

struct WaveformMeasurements
{
    float rms;
    float peakEnergy;
    float zcr;
    float specCentroid;
    float specCrest;
    float specFlat;
    float specRolloff;
    float specKurtosis;
};

#endif