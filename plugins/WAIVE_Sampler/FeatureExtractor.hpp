#ifndef FEATURE_EXTRACTOR_HPP_INCLUDED
#define FEATURE_EXTRACTOR_HPP_INCLUDED

#include "Gist.h"
// #include "Spectrogram.h"
#include <vector>

class FeatureExtractor
{
public:
    FeatureExtractor(int sampleRate, int frameSize, int hopLength, int nMels, WindowType window);

    std::vector<std::vector<float>> getMelSpectrogram(std::vector<float> *buffer, bool centered = true);

private:
    int sampleRate, frameSize, hopLength;
    int nMels;

    Gist<float> gist;
    MFCC<float> mfcc;
};

#endif