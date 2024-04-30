#ifndef WAIVESAMPLER_PARAMS_H_INCLUDED
#define WAIVESAMPLER_PARAMS_H_INCLUDED

#ifdef LINUX
#define CACHE_DIR ".cache/WAIVE"
#define SEP "/"
#elif APPLE
#define CACHE_DIR "Library/Application Support/com.thunderboomrecords.waive"
#define SEP "/"
#elif WIN32
#define CACHE_DIR "AppData\\Local\\WAIVE"
#define SEP "\\"
#endif

enum Parameters {
    kVolume0 = 0,
    kParameterCount
};

enum Updates {
    kSampleLoading = 1000,
    kSampleLoaded
};

#endif