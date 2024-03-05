
#include "WAIVEMidi.hpp"


START_NAMESPACE_DISTRHO

WAIVEMidi::WAIVEMidi() : Plugin(kParameterCount, 0, 0),
                         fThreshold(0.7)
{

}

void WAIVEMidi::initParameter(uint32_t index, Parameter &parameter)
{
    switch(index)
    {
        case kThreshold:
            parameter.name = "Threshold";
            parameter.symbol = "threshold";
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
            parameter.ranges.def = 0.7f;
            parameter.hints = kParameterIsAutomatable;
            break;
        default:
            break;
    }
}

float WAIVEMidi::getParameterValue(uint32_t index) const
{
    float val = 0.0f;
    switch(index)
    {
        case kThreshold:
            val = fThreshold;
            break;
        default:
            break;
    }

    return val;
}

void WAIVEMidi::setParameterValue(uint32_t index, float value)
{
    switch(index)
    {
        case kThreshold:
            fThreshold = value;
            break;
        default:
            break;
    }
}

void WAIVEMidi::setState(const char *key, const char *value){}

String WAIVEMidi::getState(const char *key) const
{
    String retString = String("undefined state");
    return retString;
}

void WAIVEMidi::initState(unsigned int index, String &stateKey, String &defaultStateValue)
{
    switch(index)
    {
        default:
        break;
    }
}

void WAIVEMidi::run(
    const float **,              // incoming audio
    float **,                    // outgoing audio
    uint32_t numFrames,          // size of block to process
    const MidiEvent *midiEvents, // MIDI pointer
    uint32_t midiEventCount      // Number of MIDI events in block
)
{

}

void WAIVEMidi::sampleRateChanged(double newSampleRate) {}

Plugin *createPlugin()
{
    return new WAIVEMidi();
}

END_NAMESPACE_DISTRHO