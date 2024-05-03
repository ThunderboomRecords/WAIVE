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
                               fSampleVolume(1.0f),
                               fSamplePitch(1.0f),
                               fSampleLoaded(false),
                               fSampleLength(0),
                               fSamplePtr(0),
                               fSourceLoaded(false),
                               fCurrentSample(nullptr)
{
    if (isDummyInstance())
    {
        std::cout << "** dummy instance" << std::endl;
    }

    srand(time(NULL));

    // Get and create the directory where samples and sound files will
    // be saved to
    fs::path homedir = get_homedir();
    fCacheDir = homedir / DATA_DIR;

    std::cout << "homedir: " << homedir << std::endl;
    std::cout << "cacheDir: " << fCacheDir << std::endl;

    bool result = fs::create_directory(fCacheDir);
    std::cout << "fCacheDir created: " << (result ? "true" : "false") << std::endl;

    if (result)
    {
        // newly created cache, make source folder
        fs::create_directory(fCacheDir / SOURCE_DIR);
        fs::create_directory(fCacheDir / SAMPLE_DIR);
    }

    db = new SampleDatabase((fCacheDir / "waive_sampler.db").string());
    fAllSamples = db->getAllSamples();

    stretch.presetDefault(1, (int)sampleRate);
}

WAIVESampler::~WAIVESampler()
{
    std::cout << "closing WAIVESampler..." << std::endl;
}

void WAIVESampler::initParameter(uint32_t index, Parameter &parameter)
{
    switch (index)
    {
    case kSampleVolume:
        parameter.name = "Sample Volume";
        parameter.symbol = "sampleVolume";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.8f;
        parameter.hints = kParameterIsAutomatable;
        break;
    case kSamplePitch:
        parameter.name = "Sample Pitch";
        parameter.symbol = "samplePitch";
        parameter.ranges.min = 0.1f;
        parameter.ranges.max = 4.0f;
        parameter.ranges.def = 1.0f;
        parameter.hints = kParameterIsAutomatable;
    default:
        break;
    }
}

float WAIVESampler::getParameterValue(uint32_t index) const
{
    float val = 0.0f;
    switch (index)
    {
    case kSampleVolume:
        val = fSampleVolume;
        break;
    case kSamplePitch:
        val = fSamplePitch;
        break;
    default:
        break;
    }

    return val;
}

void WAIVESampler::setParameterValue(uint32_t index, float value)
{
    switch (index)
    {
    case kSampleVolume:
        fSampleVolume = value;
        fCurrentSample->volume = value;
        renderSample();
        break;
    case kSamplePitch:
        fSamplePitch = value;
        fCurrentSample->pitch = value;
        repitchSample();
        break;
    default:
        break;
    }
}

void WAIVESampler::setState(const char *key, const char *value)
{
    if (strcmp(key, "filename") == 0)
    {
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
    switch (index)
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
    if (!fSampleLoaded)
    {
        for (uint32_t i = 0; i < numFrames; i++)
        {
            outputs[0][i] = 0.0f;
            outputs[1][i] = 0.0f;
        }
    }
    else
    {
        for (uint32_t i = 0; i < numFrames; i++)
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

    fSourceLoaded = false;
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

    if (sampleChannels > 1)
    {
        for (int i = 0; i < sampleLength; i++)
        {
            buffer->operator[](i) = (sample[i * sampleChannels] + sample[i * sampleChannels + 1]) * 0.5f;
        }
    }
    else
    {
        for (int i = 0; i < sampleLength; i++)
        {
            buffer->operator[](i) = sample[i];
        }
    }

    fSourceLoaded = true;
    fSourceFilepath = std::string(fp);
    if(fCurrentSample != nullptr)
    {
        fCurrentSample->source = fSourceFilepath;
    }
    addToUpdateQueue(kSourceLoaded);

    return true;
}

bool WAIVESampler::saveWaveform(const char *fp, float *buffer, sf_count_t size)
{
    std::cout << "WAIVESampler::saveWaveform" << std::endl;

    SndfileHandle file = SndfileHandle(
        fp,
        SFM_WRITE,
        SF_FORMAT_WAV | SF_FORMAT_PCM_16,
        1,
        sampleRate);

    file.write(buffer, size);
    std::cout << "waveform saved to " << fp << std::endl;

    return true;
}

void WAIVESampler::addToLibrary()
{
    std::cout << "WAIVESampler::addToLibrary" << std::endl;
    if (!fSampleLoaded || fCurrentSample == nullptr)
        return;

    bool result = false;
    if (fCurrentSample->saved)
    {
        result = db->updateSample(*fCurrentSample);
    }
    else
    {
        result = db->insertSample(*fCurrentSample);
    }

    if (result)
    {
        saveWaveform((fs::path(fCurrentSample->path) / fCurrentSample->name).c_str(), &fSample[0], fSample.size());
        fCurrentSample->saved = true;
    }
}

void WAIVESampler::newSample()
{
    std::cout << "WAIVESampler::newSample" << std::endl;
    time_t current_time = time(NULL);
    std::string name = fmt::format("{:d}.wav", current_time);

    fCurrentSample = new SampleInfo(current_time, name, fCacheDir / SAMPLE_DIR, true);
    fCurrentSample->pitch = fSamplePitch;
    fCurrentSample->volume = fSampleVolume;
    fCurrentSample->source = fSourceFilepath;
}

void WAIVESampler::loadSample(SampleInfo *s)
{
    fCurrentSample = s;
    if (!s->waive)
    {
        // TODO: load waveform, hide/reset controls
        return;
    }

    loadWaveform(s->source.c_str(), &fSourceWaveform);
    setParameterValue(kSamplePitch, s->pitch);
    setParameterValue(kSampleVolume, s->volume);
    selectSample(&fSourceWaveform, s->sourceStart, s->sourceEnd);
}

void WAIVESampler::selectSample(std::vector<float> *source, uint start, uint end)
{
    // TODO: ideally on separate thread (with mutex lock on fSample)

    if (start == end)
        return;

    if (start > end)
    {
        uint tmp = end;
        end = start;
        start = tmp;
    }

    if (fCurrentSample == nullptr)
    {
        newSample();
    }

    if (end >= source->size())
        end = source->size() - 1;

    fSampleLoaded = false;
    addToUpdateQueue(kSampleLoading);

    fSampleLength = end - start;
    fSampleRaw.resize(fSampleLength);
    fSamplePitched.resize(fSampleLength);
    fSample.resize(fSampleLength);

    // normalise selection to [-1.0, 1.0]
    auto minmax = std::minmax_element(source->begin() + start, source->begin() + end);
    float normaliseRatio = std::max(-(*minmax.first), *minmax.second);
    if (std::abs(normaliseRatio) <= 0.0001f)
        normaliseRatio = 1.0f;

    for (int i = 0; i < fSampleLength; i++)
    {
        fSampleRaw[i] = source->operator[](start + i) / normaliseRatio;
    }

    fSampleLoaded = true;

    fSamplePtr = 0;

    // tWaveShaping = new std::thread(&WAIVESampler::repitchSample, this);
    repitchSample();
}

void WAIVESampler::repitchSample()
{
    if (!fSampleLoaded)
        return;

    // create working buffers
    std::vector<std::vector<float>> inBuffer{{0.0f}};
    std::vector<std::vector<float>> outBuffer{{0.0f}};

    stretch.reset();

    std::cout << "stretch latency: in " << stretch.inputLatency() << "  out " << stretch.outputLatency() << std::endl;

    float startFactor = 4.0f;
    float endFactor = fSamplePitch;

    int blockSize = 256;

    // pitch sample with varying pitch
    inBuffer[0].resize(blockSize);
    outBuffer[0].resize(blockSize);

    std::vector<float> result{};
    result.resize(fSampleLength + stretch.inputLatency() + stretch.outputLatency());

    int blockstart = 0;
    float st = startFactor;

    while (blockstart < fSampleLength)
    {
        for (int i = 0; i < blockSize; i++)
        {
            if (blockstart + i >= fSampleLength)
                break;

            inBuffer[0][i] = fSampleRaw[blockstart + i];
        }

        stretch.setTransposeFactor(st);
        stretch.process(inBuffer, blockSize, outBuffer, blockSize);

        for (int i = 0; i < blockSize; i++)
        {
            if (blockstart + i >= fSampleLength)
                break;
            result[blockstart + i] = outBuffer[0][i];
        }

        blockstart += blockSize;

        st = endFactor + (startFactor - endFactor) * (1.0f - (float)blockstart / (blockSize * 20));
        st = std::clamp(st, endFactor, startFactor);
    }

    for (int i = 0; i < fSampleLength; i++)
    {
        fSamplePitched[i] = result[i + stretch.inputLatency() + stretch.outputLatency()];
    }

    renderSample();
}

void WAIVESampler::renderSample()
{
    if (!fSampleLoaded)
        return;

    for (int i = 0; i < fSampleLength; i++)
    {
        fSample[i] = fSamplePitched[i] * fSampleVolume;
    }

    getEmbeding();

    addToUpdateQueue(kSampleUpdated);
}

void WAIVESampler::getEmbeding()
{
    // TODO: placeholder is random for now
    float embedX = (float)(rand() % 10000) / 10000.0f;
    float embedY = (float)(rand() % 10000) / 10000.0f;

    fCurrentSample->embedX = embedX;
    fCurrentSample->embedY = embedY;
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
        fmax);

    printf(" ** melspec:\n");
    printf(" %.d rows, %d cols\n", melspec.size(), melspec[0].size());
    printf(" value at (4, 5) = %.4f\n", melspec[4][5]);
    std::cout << std::endl;
}

void WAIVESampler::addToUpdateQueue(int ev)
{
    updateQueue.push(ev);

    // make sure the queue does not get too long..
    while (updateQueue.size() > 64)
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