#include "WAIVESampler.hpp"

START_NAMESPACE_DISTRHO


WAIVESampler::WAIVESampler() : Plugin(kParameterCount, 0, 0),
                               fVolume0(0.0f)
{
    sampleRate = getSampleRate();
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


void WAIVESampler::setState(const char *key, const char *value){ }


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