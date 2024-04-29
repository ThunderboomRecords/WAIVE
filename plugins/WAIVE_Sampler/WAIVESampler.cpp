#include "WAIVESampler.hpp"

START_NAMESPACE_DISTRHO


WAIVESampler::WAIVESampler() : Plugin(kParameterCount, 0, 0),
                               fVolume0(0.0f),
                               fSampleLoaded(false),
                               sampleRate(getSampleRate())
{

}


void WAIVESampler::initParameter(uint32_t index, Parameter &parameter)
{
    switch(index)
    {
        case kVolume0:
            parameter.name = "Volume0";
            parameter.symbol = "volume0";
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
            parameter.ranges.def = 0.8f;
            parameter.hints = kParameterIsAutomatable;
            break;
        default:
            break;
    }
}


float WAIVESampler::getParameterValue(uint32_t index) const
{
    float val = 0.0f;
    switch(index)
    {
        case kVolume0:
            val = fVolume0;
            break;
        default:
            break;
    }

    return val;
}


void WAIVESampler::setParameterValue(uint32_t index, float value)
{
    switch(index)
    {
        case kVolume0:
            fVolume0 = value;
            break;
        default:
            break;
    }
}


void WAIVESampler::setState(const char *key, const char *value)
{
    if(strcmp(key, "filename") == 0)
    {
        fFilepath = std::string(value);
        loadSample(value);
    }
}


String WAIVESampler::getState(const char *key) const
{
    String retString = String("undefined state");
    return retString;
}


void WAIVESampler::initState(unsigned int index, String &stateKey, String &defaultStateValue)
{
    switch(index)
    {
        default:
        break;
    }
}


void WAIVESampler::run(
    const float **,              // incoming audio
    float **outputs,             // outgoing audio
    uint32_t numFrames,          // size of block to process
    const MidiEvent *midiEvents, // MIDI pointer
    uint32_t midiEventCount      // Number of MIDI events in block
)
{
    for(uint32_t i = 0; i < numFrames; i++)
    {
        outputs[0][i] = 0.0f;
        outputs[1][i] = 0.0f;
    }
}


int WAIVESampler::loadSample(const char *fp)
{
    printf("loadSample %s\n", fp);

    updateQueue.push(kSampleLoading);

    SndfileHandle fileHandle(fp);
    int sampleLength = fileHandle.frames() - 1;

    if (sampleLength == -1)
    {
        printf("Error: Unable to open input file '%s'\n", fp);
        return 0;
    }

    fSampleLength = sampleLength;
    int sampleChannels = fileHandle.channels();

    std::vector<float> sample;
    sample.resize(sampleLength * sampleChannels);
    fileHandle.read(&sample.at(0), sampleLength * sampleChannels);

    fWaveform.resize(fSampleLength);

    if(sampleChannels > 1){
        for (int i = 0; i < fSampleLength; i++) {
            fWaveform[i] = (sample[i * sampleChannels] + sample[i * sampleChannels + 1]) * 0.5f;
        }
    } else {
        for (int i = 0; i < fSampleLength; i++) {
            fWaveform[i] = sample[i];
        }
    }

    fSampleLoaded = true;

    updateQueue.push(kSampleLoaded);

    // analyseWaveform();

    return 0;
};


void WAIVESampler::analyseWaveform()
{

    int n_fft = 1024;
    int n_hop = 441;
    std::string window = "hann";
    bool center = true;
    std::string pad_mode = "constant";
    float power = 2.f;
    int n_mel = 64;
    int fmin = 0;
    int fmax = 22050;

    std::vector<std::vector<float>> melspec = librosa::Feature::melspectrogram(
        fWaveform, 
        22050, 
        n_fft, 
        n_hop, 
        window, 
        center, 
        pad_mode, 
        power,
        n_mel, 
        fmin, 
        fmax
    );

    printf(" ** melspec:\n");
    printf(" %.d rows, %d cols\n", melspec.size(), melspec[0].size());
    printf(" value at (4, 5) = %.4f\n", melspec[4][5]);
    std::cout << std::endl;
}


void WAIVESampler::sampleRateChanged(double newSampleRate)
{
    sampleRate = newSampleRate;
}


Plugin *createPlugin()
{
    return new WAIVESampler();
}


END_NAMESPACE_DISTRHO