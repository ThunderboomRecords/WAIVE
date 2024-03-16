#ifndef WAIVEPARAMS_H_INCLUDED
#define WAIVEPARAMS_H_INCLUDED

enum Parameters {
    kThreshold = 0,
    kSixteenth,
    kParameterCount
};

// enum MidiDrumNotes {
//     KD = 0x24,  // 36
//     SD = 0x26,  // 38
//     HH = 0x2A,  // 42
//     CL = 0x27,  // 39
//     TH = 0x2B   // 43
// };

// const uint8_t MidiMap[5] = {KD, SD, HH, CL, TH};
static uint8_t midiMap[9] = {36, 38, 47, 50, 43, 42, 46, 51, 49};

struct Note {
    int tick;
    uint8_t velocity;
    uint8_t midiNote;
    uint8_t channel;
    bool noteOn;
};


#endif