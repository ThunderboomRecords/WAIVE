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
                               fSamplePitchedCached(false),
                               fSampleLength(0),
                               fNormalisationRatio(1.0f),
                               fSamplePtr(0),
                               fSourceLoaded(false),
                               fCurrentSample(nullptr),
                               ampEnvGen(sampleRate, ENV_TYPE::ADSR, fAmpADSRParams)
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

    fs::create_directory(fCacheDir / SOURCE_DIR);
    fs::create_directory(fCacheDir / SAMPLE_DIR);

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
        break;
    case kAmpAttack:
        parameter.name = "Amp Attack";
        parameter.symbol = "ampAttack";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 500.0f;
        parameter.ranges.def = 10.0f;
        parameter.hints = kParameterIsAutomatable;
        break;
    case kAmpDecay:
        parameter.name = "Amp Decay";
        parameter.symbol = "ampDecay";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 500.0f;
        parameter.ranges.def = 50.0f;
        parameter.hints = kParameterIsAutomatable;
        break;
    case kAmpSustain:
        parameter.name = "Amp Sustain";
        parameter.symbol = "ampSustain";
        parameter.ranges.min = 0.1f;
        parameter.ranges.max = 4.0f;
        parameter.ranges.def = 1.0f;
        parameter.hints = kParameterIsAutomatable;
        break;
    case kAmpRelease:
        parameter.name = "Amp Release";
        parameter.symbol = "ampRelease";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 500.0f;
        parameter.ranges.def = 100.0f;
        parameter.hints = kParameterIsAutomatable;
        break;
    case kSustainLength:
        parameter.name = "Sustain Length";
        parameter.symbol = "sustainLength";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 500.0f;
        parameter.ranges.def = 1.0f;
        parameter.hints = kParameterIsAutomatable;
        break;
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
    case kAmpAttack:
        val = fAmpADSRParams.attack;
        break;
    case kAmpDecay:
        val = fAmpADSRParams.decay;
        break;
    case kAmpSustain:
        val = fAmpADSRParams.sustain;
        break;
    case kAmpRelease:
        val = fAmpADSRParams.release;
        break;
    case kSustainLength:
        val = fSustainLength;
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
        if (fCurrentSample != nullptr)
            fCurrentSample->volume = value;
        renderSample();
        break;
    case kSamplePitch:
        fSamplePitch = value;
        if (fCurrentSample != nullptr)
            fCurrentSample->pitch = value;
        renderSample();
        break;
    case kAmpAttack:
        fAmpADSRParams.attack = value;
        ampEnvGen.setADSR(fAmpADSRParams);
        renderSample();
        break;
    case kAmpDecay:
        fAmpADSRParams.decay = value;
        ampEnvGen.setADSR(fAmpADSRParams);
        renderSample();
        break;
    case kAmpSustain:
        fAmpADSRParams.sustain = value;
        ampEnvGen.setADSR(fAmpADSRParams);
        renderSample();
        break;
    case kAmpRelease:
        fAmpADSRParams.release = value;
        ampEnvGen.setADSR(fAmpADSRParams);
        renderSample();
        break;
    case kSustainLength:
        fSustainLength = value;
        renderSample();
    default:
        break;
    }
}

void WAIVESampler::setState(const char *key, const char *value)
{
    if (strcmp(key, "filename") == 0)
    {
        loadSource(value);
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

void WAIVESampler::loadSource(const char *fp)
{
    fSourceLoaded = false;
    addToUpdateQueue(kSourceLoading);

    bool result = loadWaveform(fp, &fSourceWaveform);

    if (result)
    {
        fSourceLoaded = true;
        fSourceFilepath = std::string(fp);
        fSourceLength = fSourceWaveform.size();
        if (fCurrentSample != nullptr)
        {
            fCurrentSample->source = fSourceFilepath;
        }
        addToUpdateQueue(kSourceLoaded);
    }
}

bool WAIVESampler::loadWaveform(const char *fp, std::vector<float> *buffer)
{
    printf("WAIVESampler::loadWaveform %s\n", fp);

    SndfileHandle fileHandle(fp);
    int sampleLength = fileHandle.frames();

    if (sampleLength == 0)
    {
        std::cerr << "Error: Unable to open input file " << fp << std::endl;
        return false;
    }

    int sampleChannels = fileHandle.channels();
    int fileSampleRate = fileHandle.samplerate();

    std::cout << "sampleChannels: " << sampleChannels << " "
              << " fileSampleRate: " << fileSampleRate
              << " (sampleRate: " << sampleRate << ")\n";

    std::vector<float> sample;
    sample.resize(sampleLength * sampleChannels);
    fileHandle.read(&sample.at(0), sampleLength * sampleChannels);

    std::vector<float> sample_tmp;

    // resample data
    if (fileSampleRate != sampleRate)
    {
        int new_size = sampleLength * sampleRate / fileSampleRate;
        sample_tmp.resize(new_size);

        SRC_DATA src_data;
        src_data.data_in = &sample.at(0);
        src_data.input_frames = sampleLength;
        src_data.data_out = &sample_tmp.at(0);
        src_data.output_frames = new_size;
        src_data.src_ratio = sampleRate / fileSampleRate;

        int result = src_simple(&src_data, SRC_SINC_BEST_QUALITY, sampleChannels);
        if (result != 0)
        {
            std::cerr << "Failed to convert sample rate: " << src_strerror(result) << std::endl;
        }
        else
        {
            std::cout << "sample rate conversion complete. ratio: " << src_data.src_ratio << std::endl;
        }
    }
    else
    {
        sample_tmp = sample;
    }

    buffer->resize(sampleLength);

    if (sampleChannels > 1)
    {
        for (int i = 0; i < sampleLength; i++)
        {
            buffer->operator[](i) = (sample_tmp[i * sampleChannels] + sample_tmp[i * sampleChannels + 1]) * 0.5f;
        }
    }
    else
    {
        for (int i = 0; i < sampleLength; i++)
        {
            buffer->operator[](i) = sample_tmp[i];
        }
    }

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
    if (!fSampleLoaded || fCurrentSample == nullptr)
        return;

    bool result = false;
    fCurrentSample->adsr = fAmpADSRParams;
    fCurrentSample->sustainTime = fSustainLength;
    fCurrentSample->sourceStart = fSampleStart;

    if (fCurrentSample->saved)
        result = db->updateSample(*fCurrentSample);
    else
        result = db->insertSample(*fCurrentSample);

    if (result)
    {
        saveWaveform((fs::path(fCurrentSample->path) / fCurrentSample->name).c_str(), &fSample[0], fSample.size());
        fCurrentSample->saved = true;

        fAllSamples.push_back(*fCurrentSample);
        addToUpdateQueue(kSampleAdded);

        newSample();
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
    fCurrentSample->adsr = fAmpADSRParams;

    getEmbeding();
}

void WAIVESampler::loadSample(int id)
{
    for (int i = 0; i < fAllSamples.size(); i++)
    {
        if (fAllSamples[i].getId() == id)
        {
            loadSample(&fAllSamples[i]);
            return;
        }
    }
}

void WAIVESampler::loadSample(SampleInfo *s)
{
    LOG_LOCATION
    // save/update current sample
    addToLibrary();

    fCurrentSample = s;
    fSampleLoaded = false;
    fSamplePtr = 0;

    // load source (if avaliable)
    if (fCurrentSample->waive)
    {
        loadSource(fCurrentSample->source.c_str());
        setParameterValue(kSampleVolume, fCurrentSample->volume);
        setParameterValue(kSamplePitch, fCurrentSample->pitch);
        setParameterValue(kAmpAttack, fCurrentSample->adsr.attack);
        setParameterValue(kAmpDecay, fCurrentSample->adsr.decay);
        setParameterValue(kAmpSustain, fCurrentSample->adsr.sustain);
        setParameterValue(kAmpRelease, fCurrentSample->adsr.release);
        setParameterValue(kSustainLength, fCurrentSample->sustainTime);
        fSampleStart = fCurrentSample->sourceStart;
    }

    // load sample directly (hide sample controls?)
    fs::path fp = fs::path(fCurrentSample->path) / fCurrentSample->name;
    loadWaveform(fp.c_str(), &fSample);

    fSampleLoaded = true;
    // addToUpdateQueue(kSampleLoaded);
    addToUpdateQueue(kParametersChanged);

    renderSample();
}

void WAIVESampler::selectWaveform(std::vector<float> *source, uint start, bool process = true)
{
    LOG_LOCATION

    if (fCurrentSample == nullptr)
        newSample();

    fSampleStart = start;
    fSampleLoaded = true;

    fSamplePtr = 0;
    renderSample();
    addToUpdateQueue(kSampleUpdated);
}

void WAIVESampler::repitchSample(bool bypass)
{
    std::cout << "WAIVESampler::repitchSample" << std::endl;
    fSamplePitched.resize(fSampleLength);

    if (bypass)
    {
        for (int i = 0; i < fSampleLength; i++)
        {
            fSamplePitched[i] = fSourceWaveform[fSampleStart + i];
        }
        return;
    }

    // create working buffers
    std::vector<std::vector<float>> inBuffer{{0.0f}};
    std::vector<std::vector<float>> outBuffer{{0.0f}};

    stretch.reset();

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

            inBuffer[0][i] = fSourceWaveform[fSampleStart + blockstart + i];
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

    fSamplePitchedCached = true;
}

void WAIVESampler::renderSample()
{
    if (!fSampleLoaded)
        return;

    fSampleLength = ampEnvGen.getLength(fSustainLength);
    fSampleLength = std::min(fSampleLength, (int)fSourceWaveform.size() - fSampleStart);

    auto minmax = std::minmax_element(&fSourceWaveform[fSampleStart], &fSourceWaveform[fSampleStart + fSampleLength]);
    float normaliseRatio = std::max(-(*minmax.first), *minmax.second);
    if (std::abs(normaliseRatio) <= 0.0001f)
        normaliseRatio = 1.0f;

    if (fSample.size() < fSampleLength)
        fSample.resize(fSampleLength);

    ampEnvGen.reset();
    ampEnvGen.trigger();

    float amp = ampEnvGen.getValue();
    float delta = fSamplePitch;
    float y = 0.0f;
    int index = 0;
    float indexF = 0.0f;

    for (int i = 0; i < fSampleLength; i++)
    {
        index = (int)indexF;
        y = fSourceWaveform[fSampleStart + index];
        fSample[i] = std::clamp(y * fSampleVolume * amp / normaliseRatio, -1.0f, 1.0f);

        indexF += delta;

        if (1000.f * indexF / sampleRate >= fAmpADSRParams.attack + fAmpADSRParams.decay + fSustainLength && ampEnvGen.getStage() != ADSR_Stage::RELEASE)
            ampEnvGen.release();
        ampEnvGen.process();

        if (!ampEnvGen.active)
            break;

        amp = ampEnvGen.getValue();
    }

    // getEmbeding();
    addToUpdateQueue(kSampleUpdated);
}

void WAIVESampler::getEmbeding()
{
    // TODO: placeholder is random for now
    float embedX = (float)(rand() % 10000) / 5000.0f - 1.0f;
    float embedY = (float)(rand() % 10000) / 5000.0f - 1.0f;

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

    while (updateQueue.size() > 64)
        updateQueue.pop();
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