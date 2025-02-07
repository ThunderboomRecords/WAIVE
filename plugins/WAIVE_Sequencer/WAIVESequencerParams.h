#ifndef WAIVE_SEQUENCER_PARAMS_H_INCLUDED
#define WAIVE_SEQUENCER_PARAMS_H_INCLUDED

#include <cstdint>

enum Parameters
{
    kThreshold = 0,
    kGrooveNew,
    kGrooveVar,
    kScoreNew,
    kScoreVar,
    kScoreGenre,
    kGrooveGenre,
    kThreshold1,
    kThreshold2,
    kThreshold3,
    kThreshold4,
    kThreshold5,
    kThreshold6,
    kThreshold7,
    kThreshold8,
    kThreshold9,
    kMidi1,
    kMidi2,
    kMidi3,
    kMidi4,
    kMidi5,
    kMidi6,
    kMidi7,
    kMidi8,
    kMidi9,
    kParameterCount
};

enum WAIVESequencerStates
{
    kStateScoreZ,
    kStateGrooveZ,
    kStateDrumPattern,
    kStateGeneratedTriggers,
    kStateUserTriggers,
    kStateCount
};

static const char *parameterIndexToString(uint32_t index)
{
    switch (index)
    {
    case kThreshold:
        return "kThreshold";
    case kGrooveNew:
        return "kGrooveNew";
    case kGrooveVar:
        return "kGrooveVar";
    case kScoreNew:
        return "kScoreNew";
    case kScoreVar:
        return "kScoreVar";
    case kScoreGenre:
        return "kScoreGenre";
    case kGrooveGenre:
        return "kGrooveGenre";
    case kThreshold1:
        return "kThreshold1";
    case kThreshold2:
        return "kThreshold2";
    case kThreshold3:
        return "kThreshold3";
    case kThreshold4:
        return "kThreshold4";
    case kThreshold5:
        return "kThreshold5";
    case kThreshold6:
        return "kThreshold6";
    case kThreshold7:
        return "kThreshold7";
    case kThreshold8:
        return "kThreshold8";
    case kThreshold9:
        return "kThreshold9";
    case kMidi1:
        return "kMidi1";
    case kMidi2:
        return "kMidi2";
    case kMidi3:
        return "kMidi3";
    case kMidi4:
        return "kMidi4";
    case kMidi5:
        return "kMidi5";
    case kMidi6:
        return "kMidi6";
    case kMidi7:
        return "kMidi7";
    case kMidi8:
        return "kMidi8";
    case kMidi9:
        return "kMidi9";
    case kParameterCount:
        return "kParameterCount";
    }

    return "UNKNOWN Parameter";
}

#endif