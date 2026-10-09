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

std::vector<std::vector<float>> FeatureExtractor::getMelSpectrogram(const std::vector<float> *buffer, bool centered, int maxFrames)
{
    std::vector<std::vector<float>> melspec;

    // Frame i covers input samples [i * hopLength - pad, i * hopLength - pad + frameSize),
    // so this many samples give the first maxFrames frames exactly
    size_t inputLength = buffer->size();
    if (maxFrames > 0)
    {
        size_t needed = static_cast<size_t>(maxFrames - 1) * hopLength + (centered ? frameSize - frameSize / 2 : frameSize);
        inputLength = std::min(inputLength, needed);
    }
    std::vector<float> x(buffer->begin(), buffer->begin() + inputLength);

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
    for (int i = 0; i + frameSize < x.size(); i += hopLength)
    {
        if (maxFrames > 0 && static_cast<int>(melspec.size()) >= maxFrames)
            break;

        gist.processAudioFrame(&x[i], frameSize);
        std::vector<float> magSpectrum = gist.getMagnitudeSpectrum();
        // for (int j = 0; j < magSpectrum.size(); j++)
        //     magSpectrum[j] = std::powf(magSpectrum[j], 2.0f);

        mfcc.calculateMelFrequencySpectrum(magSpectrum);
        melspec.push_back(std::vector<float>(mfcc.melSpectrum.begin(), mfcc.melSpectrum.end() - 1));
    }

    return melspec;
}
