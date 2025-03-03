#ifndef WAIVE_SAMPLER_PARAMS_H_INCLUDED
#define WAIVE_SAMPLER_PARAMS_H_INCLUDED

#ifndef LOG_LOCATION
#define LOG_LOCATION std::cout << __func__ << "():  " << __FILE__ << ":" << __LINE__ << std::endl;
#endif

// DATA_DIR: Data is stored in folder Poco::Path::dataDir()
// e.g. ~/.local/share/WAIVE on UNIX machines and on Windows
// typically C:\Users\user\AppData\Local\WAIVE
#define DATA_DIR "WAIVE";

// SOURCE_DIR: folder in DATA_DIR that holds the source
// audio files.
#define SOURCE_DIR "Sources"

// SAMPLE_DIR: folder in DATA_DIR that contains saved
// sample waveforms
#define SAMPLE_DIR "Samples"

// Number of SampleSlots
#define NUM_SLOTS 18

enum Parameters
{
    kSampleVolume = 0,
    kSamplePitch,
    kAmpAttack,
    kAmpDecay,
    kAmpSustain,
    kAmpRelease,
    kSustainLength,
    kPercussiveBoost,
    kFilterCutoff,
    kFilterResonance,
    kFilterType,
    kSlot1MidiNumber,
    kSlot2MidiNumber,
    kSlot3MidiNumber,
    kSlot4MidiNumber,
    kSlot5MidiNumber,
    kSlot6MidiNumber,
    kSlot7MidiNumber,
    kSlot8MidiNumber,
    kSlot9MidiNumber,
    kSlot10MidiNumber,
    kSlot11MidiNumber,
    kSlot12MidiNumber,
    kSlot13MidiNumber,
    kSlot14MidiNumber,
    kSlot15MidiNumber,
    kSlot16MidiNumber,
    kSlot17MidiNumber,
    kSlot18MidiNumber,
    kSlot1Gain,
    kSlot2Gain,
    kSlot3Gain,
    kSlot4Gain,
    kSlot5Gain,
    kSlot6Gain,
    kSlot7Gain,
    kSlot8Gain,
    kSlot9Gain,
    kSlot10Gain,
    kSlot11Gain,
    kSlot12Gain,
    kSlot13Gain,
    kSlot14Gain,
    kSlot15Gain,
    kSlot16Gain,
    kSlot17Gain,
    kSlot18Gain,
    kSlot1Pan,
    kSlot2Pan,
    kSlot3Pan,
    kSlot4Pan,
    kSlot5Pan,
    kSlot6Pan,
    kSlot7Pan,
    kSlot8Pan,
    kSlot9Pan,
    kSlot10Pan,
    kSlot11Pan,
    kSlot12Pan,
    kSlot13Pan,
    kSlot14Pan,
    kSlot15Pan,
    kSlot16Pan,
    kSlot17Pan,
    kSlot18Pan,
    kParameterCount
};

enum WAIVESamplerStates
{
    kStateSampleSlots,
    kStateOSC,
    kStateCount
};

#endif