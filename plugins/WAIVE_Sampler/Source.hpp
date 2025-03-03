#ifndef SOURCE_HPP_INCLUDED
#define SOURCE_HPP_INCLUDED

#include <string>
#include <vector>
#include <mutex>

#include "WaveformFeatures.hpp"

struct Source
{
    Source(const Source &other)
        : name(other.name),
          fp(other.fp),
          buffer(other.buffer),
          length(other.length),
          sourceFeatures(other.sourceFeatures),
          sourceMeasurements(other.sourceMeasurements),
          tagString(other.tagString),
          sourceLoaded(other.sourceLoaded) {};
    Source() = default;
    Source(Source &&) = delete;
    Source &operator=(Source &&) = delete;
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