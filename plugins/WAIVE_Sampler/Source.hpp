#ifndef SOURCE_HPP_INCLUDED
#define SOURCE_HPP_INCLUDED

#include <string>
#include <vector>
#include <mutex>

#include "WaveformFeatures.hpp"

// Shared between a SampleInfo and its duplicates (see SampleInfo::sourceInfo),
// so the audio is never copied
struct Source
{
    Source() = default;
    Source(const Source &) = delete;
    Source &operator=(const Source &) = delete;
    bool sourceLoaded = false;
    std::string name;
    std::string fp;
    std::vector<float> buffer;
    size_t length = 0;
    std::vector<WaveformFeature> sourceFeatures;
    std::vector<WaveformMeasurements> sourceMeasurements;
    std::mutex mtx;
    std::string tagString;
};

#endif // SOURCE_HPP_INCLUDED