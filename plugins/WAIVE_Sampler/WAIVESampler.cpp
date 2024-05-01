#include "WAIVESampler.hpp"

fs::path get_homedir()
{
    std::string homedir = "";
#ifdef WIN32
    homedir += getenv("HOMEDRIVE") + getenv("HOMEPATH");
#else
    homedir += getenv("HOME");
#endif
    return fs::path(homedir);
}


START_NAMESPACE_DISTRHO


WAIVESampler::WAIVESampler() : Plugin(kParameterCount, 0, 0),
                               sampleRate(getSampleRate()),
                               fVolume0(0.0f),
                               fSampleLoaded(false),
                               fSampleLength(0),
                               fSamplePtr(0)
{
    if(isDummyInstance())
    {
        std::cout << "** dummy instance" << std::endl;
    }

    // Get and create the directory where samples and sound files will
    // be saved to
    fs::path homedir = get_homedir();
    fCacheDir = homedir/DATA_DIR;

    std::cout << "homedir: " << homedir << std::endl;
    std::cout << "cacheDir: " << fCacheDir << std::endl;

    bool result = fs::create_directory(fCacheDir);
    std::cout << "fCacheDir created: " << (result ? "true" : "false") << std::endl;

    if(result)
    {
        // newly created cache, make database file and source folder
        fs::create_directory(fCacheDir/SOURCE_DIR);
        fs::create_directory(fCacheDir/SAMPLE_DIR);
    }


    // read database file
    io::CSVReader<3> in(fCacheDir/"db.csv");
    in.read_header(io::ignore_extra_column, "index", "data1", "data2");
    std::string data1, data2;
    int index;
    while(in.read_row(index, data1, data2))
    {
        std::cout << "index: " << index << " data1: " << data1 << " data2: " << data2 << std::endl;
    }


    stretch.presetDefault(1, (int)sampleRate);

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
        loadWaveform(value, &fSourceWaveform);
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
    if(!fSampleLoaded)
    {
        for(uint32_t i = 0; i < numFrames; i++)
        {
            outputs[0][i] = 0.0f;
            outputs[1][i] = 0.0f;
        }
    }
    else
    {
        for(uint32_t i = 0; i < numFrames; i++)
        {
            outputs[0][i] = fSample[fSamplePtr];
            outputs[1][i] = fSample[fSamplePtr];
            fSamplePtr = (fSamplePtr + 1) % fSampleLength;
        }
    }
}


bool WAIVESampler::loadWaveform(const char *fp, std::vector<float> *buffer)
{
    printf("WAIVESampler::loadWaveform %s\n", fp);

    addToUpdateQueue(kSampleLoading);

    SndfileHandle fileHandle(fp);
    int sampleLength = fileHandle.frames() - 1;

    if (sampleLength == -1)
    {
        printf("Error: Unable to open input file '%s'\n", fp);
        return false;
    }

    int sampleChannels = fileHandle.channels();

    std::vector<float> sample;
    sample.resize(sampleLength * sampleChannels);
    fileHandle.read(&sample.at(0), sampleLength * sampleChannels);

    buffer->resize(sampleLength);

    if(sampleChannels > 1){
        for (int i = 0; i < sampleLength; i++) {
            buffer->operator[](i) = (sample[i * sampleChannels] + sample[i * sampleChannels + 1]) * 0.5f;
        }
    } else {
        for (int i = 0; i < sampleLength; i++) {
            buffer->operator[](i) = sample[i];
        }
    }

    addToUpdateQueue(kSampleLoaded);

    return true;
};


void WAIVESampler::selectSample(std::vector<float> *source, uint start, uint end, std::vector<float> *destination)
{
    if(start == end) return;

    if(start > end)
    {
        uint tmp = end;
        end = start;
        start = tmp;
    }

    if(end >= source->size())
        end = source->size() - 1;

    fSampleLoaded = false;

    uint length = end - start;
    destination->resize(length);
    fSampleLength = length;

    // normalise selection to [-1.0, 1.0]
    auto minmax = std::minmax_element(source->begin()+start, source->begin()+end);
    float normaliseRatio = std::max(-(*minmax.first), *minmax.second);
    if(std::abs(normaliseRatio) <= 0.0001f) normaliseRatio = 1.0f;

    for(int i=0; i < length; i++)
    {
        destination->operator[](i) = source->operator[](start + i) / normaliseRatio;
    }

    fSampleLoaded = true;
}


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
        fSourceWaveform, 
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


void WAIVESampler::addToUpdateQueue(int ev)
{
    updateQueue.push(ev);

    // make sure the queue does not get too long..
    while(updateQueue.size() > 64)
    {
        updateQueue.pop();
    }
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