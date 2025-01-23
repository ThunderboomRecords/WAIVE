#ifndef SOURCE_HPP_INCLUDED
#define SOURCE_HPP_INCLUDED

#include <string>
#include <vector>

#include "WaveformFeatures.hpp"

struct Source
{
    bool sourceLoaded;
    std::string name;
    std::string fp;
    std::vector<float> buffer;
    size_t length;
    std::vector<WaveformFeature> sourceFeatures;
    std::vector<WaveformMeasurements> sourceMeasurements;
    std::mutex mtx;
    std::string tagString;
};

#endif // SOURCE_HPP_INCLUDED