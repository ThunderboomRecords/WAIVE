#include "FeatureExtractor.hpp"

FeatureExtractor::FeatureExtractor(
    int sampleRate,
    int frameSize,
    int hopLength,
    int nMels,
    WindowType window) : sampleRate(sampleRate),
                         frameSize(frameSize),
                         hopLength(hopLength),
                         nMels(nMels),
                         gist(frameSize, sampleRate, window),
                         mfcc(frameSize, sampleRate)
{
    mfcc.setNumCoefficients(nMels + 1);
}

std::vector<std::vector<float>> FeatureExtractor::getMelSpectrogram(const std::vector<float> *buffer, bool centered)
{
    std::vector<std::vector<float>> melspec;
    std::vector<float> x(*buffer);

    // 1. Pad input to center window, using "reflect" mode
    if (centered)
    {
        int pad_size = frameSize / 2;
        x.insert(x.begin(), pad_size, 0.f);
        x.insert(x.end(), pad_size, 0.f);
        // for (int i = 0; i < pad_size; i++)
        // {
        //     x[i] = buffer->at(pad_size - i - 1);
        //     x[x.size() - i - 1] = buffer->at(buffer->size() - pad_size + i);
        // }
    }

    // 2. Compute
    for (int i = 0; i < x.size() - frameSize; i += hopLength)
    {
        gist.processAudioFrame(std::vector<float>(x.begin() + i, x.begin() + i + frameSize));
        std::vector<float> magSpectrum = gist.getMagnitudeSpectrum();
        // for (int j = 0; j < magSpectrum.size(); j++)
        //     magSpectrum[j] = std::powf(magSpectrum[j], 2.0f);

        mfcc.calculateMelFrequencySpectrum(magSpectrum);
        melspec.push_back(std::vector<float>(mfcc.melSpectrum.begin(), mfcc.melSpectrum.end() - 1));
    }

    return melspec;
}
