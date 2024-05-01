#ifndef WAIVESAMPLER_PARAMS_H_INCLUDED
#define WAIVESAMPLER_PARAMS_H_INCLUDED

// DATA_DIR: relative from the users $HOME directory
#ifdef LINUX
#define DATA_DIR ".cache/WAIVE"
#elif APPLE
#define DATA_DIR "Library/Application Support/com.thunderboomrecords.waive"
#elif WIN32
// TODO: test on Windows! 
// - does it need backslashes, or does std::filesystem make
//   the conversion from '/'?
#define DATA_DIR "AppData\\Local\\WAIVE"
#endif

// SOURCE_DIR: folder in DATA_DIR that holds the source 
// audio files.
#define SOURCE_DIR "Sources"

// SAMPLE_DIR: folder in DATA_DIR that contains saved
// sample waveforms
#define SAMPLE_DIR "Samples"

enum Parameters {
    kVolume0 = 0,
    kParameterCount
};

enum Updates {
    kSampleLoading = 1000,
    kSampleLoaded
};

#endif