#ifndef FEATURE_EXTRACTOR_HPP_INCLUDED
#define FEATURE_EXTRACTOR_HPP_INCLUDED

#include "Gist.h"
// #include "Spectrogram.h"
#include <vector>

class FeatureExtractor
{
public:
    FeatureExtractor(int sampleRate, int frameSize, int hopLength, int nMels, WindowType window);

    // maxFrames > 0 limits the output to the first maxFrames frames, and
    // only the audio needed for those frames is processed
    std::vector<std::vector<float>> getMelSpectrogram(const std::vector<float> *buffer, bool centered = true, int maxFrames = 0);

private:
    int sampleRate, frameSize, hopLength;
    int nMels;

    Gist<float> gist;
    MFCC<float> mfcc;
};

#endif