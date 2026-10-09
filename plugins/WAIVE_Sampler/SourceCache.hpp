#ifndef SOURCE_CACHE_HPP_INCLUDED
#define SOURCE_CACHE_HPP_INCLUDED

#include <string>
#include <vector>

#include "WaveformFeatures.hpp"

// Disk cache for resampled source audio and source analysis, so loading
// the same source again skips the slow steps.
//
// Entries are keyed on the source file's path, size and modification time,
// plus a "variant" string describing how the data was made (e.g. the sample
// rate). Every function fails safely: a missing, unreadable or corrupt
// entry just means the caller does the work as usual.
namespace SourceCache
{
    // Returns the cache key for a file, or "" if the file should not be
    // cached (missing, or in the temporary folder)
    std::string makeKey(const std::string &filePath, const std::string &variant);

    bool readAudio(const std::string &key, std::vector<float> &buffer);
    void writeAudio(const std::string &key, const std::vector<float> &buffer);

    bool readAnalysis(const std::string &key, std::vector<WaveformFeature> &features, std::vector<WaveformMeasurements> &measurements);
    void writeAnalysis(const std::string &key, const std::vector<WaveformFeature> &features, const std::vector<WaveformMeasurements> &measurements);
}

#endif
