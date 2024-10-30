#include "WAIVESampler.hpp"

START_NAMESPACE_DISTRHO

uint8_t defaultMidiMap[] = {36, 38, 47, 50, 43, 42, 46, 51, 49};

ImporterTask::ImporterTask(WAIVESampler *ws, ThreadsafeQueue<std::string> *queue) : Poco::Task("ImporterTask"), _ws(ws), _queue(queue) {};

void ImporterTask::runTask()
{
    while (true)
    {
        std::optional<std::string> fp = _queue->pop();
        if (fp.has_value())
        {
            ImporterTask::import(fp.value());
            _ws->pluginUpdate.notify(&_ws, WAIVESampler::PluginUpdate::kSampleAdded);
        }
        else
            break;
    }
}

void ImporterTask::import(const std::string &fp)
{
    std::string folder = Poco::Path(fp).makeParent().toString();
    std::string name = Poco::Path(fp).getFileName();

    name.assign(_ws->sd.getNewSampleName(name));

    std::cout << "     folder: " << folder << std::endl;
    std::cout << "       name: " << name << std::endl;

    std::vector<float> sampleCopy;
    int sampleLength = loadWaveform(fp.c_str(), sampleCopy, _ws->getSampleRate());

    if (sampleLength == 0)
    {
        std::cout << "sampleLength is 0\n";
        return;
    }

    // id is assigned when added to the database
    std::shared_ptr<SampleInfo> info(new SampleInfo(-1, name, "", false));
    info->adsr.attack = 0.0f;
    info->adsr.decay = 0.0f;
    info->adsr.sustain = 1.0f;
    info->adsr.release = 0.0f;
    info->sustainLength = 1000.0f;
    info->percussiveBoost = 0.0f;
    info->filterCutoff = 0.999f;
    info->filterType = Filter::FILTER_LOWPASS;
    info->filterResonance = 0;
    info->pitch = 1.0f;
    info->volume = 1.0f;
    info->source = fp;
    info->sourceStart = 0;
    info->saved = true;
    // info->tags.push_back({"imported"});

    // TODO: render loaded sample...
    auto embedding = _ws->getEmbedding(&sampleCopy);
    info->embedX = embedding.first;
    info->embedY = embedding.second;

    _ws->sd.addToLibrary(info);

    bool result = saveWaveform(_ws->sd.getFullSamplePath(info).c_str(), &(sampleCopy.at(0)), sampleLength, _ws->getSampleRate());
    if (!result)
    {
        throw Poco::Exception("Failed to save waveform");
    }

    std::cout << " - import done\n";
};

FeatureExtractorTask::FeatureExtractorTask(WAIVESampler *ws) : Poco::Task("FeatureExtractorTask"), _ws(ws) {}

void FeatureExtractorTask::runTask()
{
    std::cout << "FeatureExtractorTask::runTask()" << std::endl;
    if (_ws->fSourceLength == 0)
        return;

    // TODO: check and load cached features
    try
    {
        Poco::File cachedir(Poco::Path(Poco::Path::cacheHome()).append("WAIVE").append("SourceAnalysis"));
        if (!cachedir.exists())
        {
            cachedir.createDirectories();
        }
    }
    catch (const Poco::Exception &e)
    {
        std::cerr << "Error creating cache directories: " << e.displayText() << std::endl;
        return;
    }

    int frameIndex = 0;
    long frame = 0;
    long length = _ws->fSourceLength;
    Gist<float> gist(128, (int)_ws->getSampleRate());
    const int frameSize = gist.getAudioFrameSize();

    std::lock_guard<std::mutex> lock(_ws->sourceFeaturesMtx);
    _ws->fSourceFeatures.clear();
    _ws->fSourceMeasurements.clear();

    float pStep = (float)gist.getAudioFrameSize() / length;
    float p = 0.0f;

    long lastOnset = -5000;

    while (frame < length - frameSize && !isCancelled())
    {
        WaveformMeasurements m;
        gist.processAudioFrame(&_ws->fSourceWaveform.at(frame), frameSize);

        m.frame = frame;
        m.rms = gist.rootMeanSquare();
        m.peakEnergy = gist.peakEnergy();
        m.specCentroid = gist.spectralCentroid();
        m.specCrest = gist.spectralCrest();
        m.specFlat = gist.spectralFlatness();
        m.specKurtosis = gist.spectralKurtosis();
        m.specRolloff = gist.spectralRolloff();
        m.zcr = gist.zeroCrossingRate();
        m.highfrequencyContent = gist.highFrequencyContent();

        // printf("  RMS: %6.2f PE: %6.2f SCent: %6.2f SCre: %6.2f SF: %6.2f SK: %6.2f SR: %6.2f ZCR: %6.2f\n",
        //        m.rms, m.peakEnergy, m.specCentroid, m.specCrest, m.specFlat, m.specKurtosis, m.specRolloff, m.zcr);

        float onset = gist.complexSpectralDifference();

        if (onset > 100.f && frame > lastOnset + 4800)
        {
            _ws->fSourceFeatures.push_back({FeatureType::Onset,
                                            "onset",
                                            onset,
                                            frame,
                                            frame,
                                            frameIndex});
            lastOnset = frame;
        }
        _ws->fSourceMeasurements.push_back(m);
        p += pStep;
        setProgress(p);

        frame += gist.getAudioFrameSize();
        frameIndex++;
    }

    // TODO: cache features here (if not cancelled)

    setProgress(1.0f);
}

WaveformLoaderTask::WaveformLoaderTask(std::shared_ptr<std::vector<float>> _buffer, std::mutex *_mutex, const std::string &_fp, int _sampleRate)
    : Poco::Task("WaveformLoaderTask"), buffer(_buffer), mutex(_mutex), fp(_fp), sampleRate(_sampleRate) {};

WaveformLoaderTask::~WaveformLoaderTask() {}

void WaveformLoaderTask::runTask()
{
    // std::cout << "WaveformLoaderTask::runTask()" << std::endl;

    SndfileHandle fileHandle(fp, SFM_READ);
    if (fileHandle.error())
    {
        std::cerr << "Error: Unable to open input file " << fp << "\n  error: " << sf_error_number(fileHandle.error()) << std::endl;
        return;
    }

    size_t sampleLength = fileHandle.frames();

    if (sampleLength == 0)
    {
        std::cerr << "Error: Unable to open input file " << fp << "\n  error: " << sf_error_number(fileHandle.error()) << std::endl;

        buffer->resize(0);
        return;
    }

    int sampleChannels = fileHandle.channels();
    int fileSampleRate = fileHandle.samplerate();

    std::vector<float> sample;
    sample.resize(sampleLength * sampleChannels);

    fileHandle.read(sample.data(), sampleLength * sampleChannels);

    std::vector<float> sample_tmp;
    size_t new_size = sampleLength;

    if (isCancelled())
        return;

    // resample data
    if (fileSampleRate == sampleRate)
    {
        sample_tmp.swap(sample);
    }
    else
    {
        double src_ratio = (double)sampleRate / fileSampleRate;
        new_size = (size_t)(sampleLength * src_ratio + 1);
        sample_tmp.reserve(new_size * sampleChannels + sampleChannels);

        SRC_DATA src_data;
        src_data.src_ratio = src_ratio;

        std::cout << "RESAMPLING WAVEFORM:\n";
        std::cout << "  fileSampleRate: " << fileSampleRate << std::endl;
        std::cout << "      sampleRate: " << sampleRate << std::endl;
        std::cout << " sample_channels: " << sampleChannels << std::endl;
        std::cout << "       src_ratio: " << src_data.src_ratio << std::endl;
        std::cout << "        new_size: " << new_size << std::endl;

        size_t chunk = 512;
        std::vector<float> outputChunk((size_t)(sampleChannels * chunk * src_data.src_ratio) + sampleChannels);

        double progress = 0.0f;
        int error;

        // Use a smart pointer to ensure src_delete is called even if an exception is thrown.
        std::unique_ptr<SRC_STATE, decltype(&src_delete)> converter(src_new(SRC_SINC_BEST_QUALITY, sampleChannels, &error), &src_delete);
        if (!converter)
        {
            std::cerr << "Could not init samplerate converter, reason: " << sf_error_number(error) << std::endl;
            return;
        }

        setProgress(progress);

        src_data.end_of_input = 0;
        src_data.input_frames = chunk;

        size_t inputPos = 0;
        double pStep = (double)(chunk * sampleChannels) / sample.size();
        while (inputPos < sample.size() && !isCancelled())
        {
            size_t currentChunkSize = std::min(chunk, (sample.size() - inputPos) / sampleChannels);

            src_data.data_in = sample.data() + inputPos;
            src_data.input_frames = currentChunkSize;
            src_data.data_out = outputChunk.data();
            src_data.output_frames = outputChunk.size() / sampleChannels;

            error = src_process(converter.get(), &src_data);
            if (error)
            {
                std::cout << "Error during sample rate conversion: " << src_strerror(error) << std::endl;
                return;
            }

            sample_tmp.insert(sample_tmp.end(), src_data.data_out, src_data.data_out + src_data.output_frames_gen * sampleChannels);

            progress += pStep;
            setProgress(progress);

            inputPos += currentChunkSize * sampleChannels;
        }
    }

    if (isCancelled())
        return;

    std::lock_guard<std::mutex> lock(*mutex);
    buffer->resize(new_size);

    // std::cout << "WaveformLoaderTask new_size: " << new_size << std::endl;

    //  TODO: mix to Mono before sample rate conversion??
    if (sampleChannels > 1)
    {
        for (size_t i = 0; i < new_size; ++i)
        {
            float sum = 0.0f;
            for (int ch = 0; ch < sampleChannels; ++ch)
                sum += sample_tmp[i * sampleChannels + ch];
            buffer->at(i) = sum / sampleChannels;
        }
    }
    else
    {
        std::copy(sample_tmp.begin(), sample_tmp.begin() + new_size, buffer->begin());
    }

    // std::cout << "WaveformLoaderTask::runTask() finished" << std::endl;
}

void SamplePlayer::clear()
{
    this->waveform = nullptr;
    this->length = 0;
    this->ptr = 0;
    this->midi = -1;
    this->gain = 1.0f;
    this->velocity = 0.8f;
    this->pitch = 60.f;
    this->pan = 0.0f;
    this->state = PlayState::STOPPED;
    this->active = false;
    this->sampleInfo = nullptr;
}

WAIVESampler::WAIVESampler() : Plugin(kParameterCount, 0, 0),
                               sampleRate(getSampleRate()),
                               fSampleLoaded(false),
                               fNormalisationRatio(1.0f),
                               fSourceLoaded(false),
                               fCurrentSample(nullptr),
                               ampEnvGen(getSampleRate(), ENV_TYPE::ADSR, {10, 50, 0.7, 100}),
                               oscClient("localhost", 8000),
                               taskManager("plugin manager", 1, 8, 60, 0),
                               converterManager("converter manager", 1, 1, 60, 0),
                               httpClient(&taskManager),
                               sd(&httpClient),
                               fe(getSampleRate(), 1024, 441, 64, WindowType::HanningWindow),
                               importerTask(new ImporterTask(this, &import_queue)),
                               waveformLoaderTask(nullptr)
{
    if (isDummyInstance())
    {
        std::cout << "** dummy instance" << std::endl;
        // return;
    }

    printf(" VERSION: %d.%d.%d\n", V_MAJ, V_MIN, V_PAT);

    // Register notifications
    taskManager.addObserver(Poco::Observer<WAIVESampler, Poco::TaskFinishedNotification>(*this, &WAIVESampler::onTaskFinished));
    converterManager.addObserver(Poco::Observer<WAIVESampler, Poco::TaskFinishedNotification>(*this, &WAIVESampler::onTaskFinished));
    sd.databaseUpdate += Poco::delegate(this, &WAIVESampler::onDatabaseChanged);
    sd.taskManager.addObserver(Poco::Observer<WAIVESampler, Poco::TaskFinishedNotification>(*this, &WAIVESampler::onTaskFinished));

    samplePlayerWaveforms.resize(11);
    for (int i = 0; i < 11; i++)
    {
        SamplePlayer sp;
        sp.waveform = &samplePlayerWaveforms[i];
        samplePlayers.push_back(sp);
    }

    editorPreviewPlayer = &samplePlayers[8];
    editorPreviewWaveform = &samplePlayerWaveforms[8];
    mapPreviewPlayer = &samplePlayers[9];
    mapPreviewWaveform = &samplePlayerWaveforms[9];
    sourcePreviewPlayer = &samplePlayers[10];
    sourcePreviewWaveform = &samplePlayerWaveforms[10];

    for (int i = 0; i < 8; i++)
        samplePlayers[i].midi = defaultMidiMap[i];

    // Load models
    // std::cout << "Loading TSNE model...\n";
    try
    {
        auto info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

        sessionOptions.SetIntraOpNumThreads(1);
        sessionOptions.SetInterOpNumThreads(1);

        mTSNE = std::make_unique<Ort::Session>(mEnv, (void *)tsne_model_onnx_start, tsne_model_onnx_size, sessionOptions);

        assert(mTSNE);

        mTSNEInputShape = GetInputShapes(mTSNE);
        mTSNEOutputShape = GetOutputShapes(mTSNE);

        mTSNEInputNames = GetInputNames(mTSNE);
        mTSNEOutputNames = GetOutputNames(mTSNE);

        mTSNEInput.resize(mTSNEInputShape[0][0]);
        mTSNEOutput.resize(mTSNEOutputShape[0][0]);

        std::fill(mTSNEInput.begin(), mTSNEInput.end(), 0.0f);
        std::fill(mTSNEOutput.begin(), mTSNEOutput.end(), 0.0f);

        mTSNEInputTensor.push_back(Ort::Value::CreateTensor<float>(info, mTSNEInput.data(), mTSNEInput.size(), mTSNEInputShape[0].data(), mTSNEInputShape[0].size()));
        mTSNEOutputTensor.push_back(Ort::Value::CreateTensor<float>(info, mTSNEOutput.data(), mTSNEOutput.size(), mTSNEOutputShape[0].data(), mTSNEOutputShape[0].size()));

        // PrintModelDetails("TSNE model", mTSNEInputNames, mTSNEOutputNames, mTSNEInputShape, mTSNEOutputShape);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading model: " << e.what() << '\n';
        return;
    }

    std::lock_guard<std::mutex> lock(tempBufferMutex);
    tempBuffer = std::make_shared<std::vector<float>>();

    // OSC Test
    oscClient.sendMessage("/WAIVE_Sampler/Started", {"WAIVESampler started"});

    // sd.checkLatestRemoteVersion();
}

WAIVESampler::~WAIVESampler()
{
    std::cout << "closing WAIVESampler..." << std::endl;

    converterManager.cancelAll();
    converterManager.joinAll();
    taskManager.cancelAll();
    taskManager.joinAll();
}

void WAIVESampler::initParameter(uint32_t index, Parameter &parameter)
{
    int slot = 0;
    parameter.hints = kParameterIsAutomatable;
    switch (index)
    {
    case kSampleVolume:
        parameter.name = "Sample Volume";
        parameter.symbol = "sampleVolume";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 2.0f;
        parameter.ranges.def = 1.0f;
        break;
    case kSamplePitch:
        parameter.name = "Sample Pitch";
        parameter.symbol = "samplePitch";
        parameter.ranges.min = 0.2f;
        parameter.ranges.max = 4.0f;
        parameter.ranges.def = 1.0f;
        break;
    case kPercussiveBoost:
        parameter.name = "Percussion Boost";
        parameter.symbol = "percussionBoost";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.3f;
        break;
    case kAmpAttack:
        parameter.name = "Amp Attack";
        parameter.symbol = "ampAttack";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 500.0f;
        parameter.ranges.def = 10.0f;
        break;
    case kAmpDecay:
        parameter.name = "Amp Decay";
        parameter.symbol = "ampDecay";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 500.0f;
        parameter.ranges.def = 50.0f;
        break;
    case kAmpSustain:
        parameter.name = "Amp Sustain";
        parameter.symbol = "ampSustain";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.7f;
        break;
    case kAmpRelease:
        parameter.name = "Amp Release";
        parameter.symbol = "ampRelease";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 500.0f;
        parameter.ranges.def = 100.0f;
        break;
    case kSustainLength:
        parameter.name = "Sustain Length";
        parameter.symbol = "sustainLength";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1000.0f;
        parameter.ranges.def = 100.0f;
        break;
    case kFilterCutoff:
        parameter.name = "Filter Cutoff";
        parameter.symbol = "filterCutoff";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 0.999f;
        parameter.ranges.def = 0.999f;
        break;
    case kFilterResonance:
        parameter.name = "Filter Resonance";
        parameter.symbol = "filterResonance";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.0f;
        break;
    case kFilterType:
        parameter.name = "Filter Type";
        parameter.symbol = "filterType";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 3.0f;
        parameter.ranges.def = 0.0f;
        parameter.hints = kParameterIsInteger;
        break;
    case kSlot1MidiNumber:
    case kSlot2MidiNumber:
    case kSlot3MidiNumber:
    case kSlot4MidiNumber:
    case kSlot5MidiNumber:
    case kSlot6MidiNumber:
    case kSlot7MidiNumber:
    case kSlot8MidiNumber:
        slot = index - kSlot1MidiNumber;
        parameter.name = fmt::format("Sample {:d} Midi Number", slot + 1).c_str();
        parameter.symbol = fmt::format("Sample{:d}Midi", slot + 1).c_str();
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 127.0f;
        parameter.ranges.def = (float)defaultMidiMap[slot];
        parameter.hints |= kParameterIsInteger;
        break;
    default:
        break;
    }
}

float WAIVESampler::getParameterValue(uint32_t index) const
{
    float val = 0.0f;
    int slot = 0;
    switch (index)
    {
    case kSampleVolume:
        if (fCurrentSample != nullptr)
            val = fCurrentSample->volume;
        break;
    case kSamplePitch:
        if (fCurrentSample != nullptr)
            val = fCurrentSample->pitch;
        break;
    case kPercussiveBoost:
        if (fCurrentSample != nullptr)
            val = fCurrentSample->percussiveBoost;
        break;
    case kAmpAttack:
        val = ampEnvGen.getAttack();
        break;
    case kAmpDecay:
        val = ampEnvGen.getDecay();
        break;
    case kAmpSustain:
        val = ampEnvGen.getSustain();
        break;
    case kAmpRelease:
        val = ampEnvGen.getRelease();
        break;
    case kSustainLength:
        if (fCurrentSample != nullptr)
            val = fCurrentSample->sustainLength;
        break;
    case kFilterCutoff:
        if (fCurrentSample != nullptr)
            val = fCurrentSample->filterCutoff;
        break;
    case kFilterResonance:
        if (fCurrentSample != nullptr)
            val = fCurrentSample->filterResonance;
        break;
    case kFilterType:
        if (fCurrentSample != nullptr)
            val = fCurrentSample->filterType;
        break;
    case kSlot1MidiNumber:
    case kSlot2MidiNumber:
    case kSlot3MidiNumber:
    case kSlot4MidiNumber:
    case kSlot5MidiNumber:
    case kSlot6MidiNumber:
    case kSlot7MidiNumber:
    case kSlot8MidiNumber:
        slot = index - kSlot1MidiNumber;
        val = (float)samplePlayers[slot].midi;
        break;
    default:
        std::cerr << "getParameter: Unknown parameter index: " << index << "  " << std::endl;
        break;
    }

    return val;
}

void WAIVESampler::setParameterValue(uint32_t index, float value)
{
    int slot = 0;
    switch (index)
    {
    case kSampleVolume:
        if (fCurrentSample != nullptr)
            fCurrentSample->volume = value;
        renderSample();
        break;
    case kSamplePitch:
        if (fCurrentSample != nullptr)
            fCurrentSample->pitch = value;
        renderSample();
        break;
    case kPercussiveBoost:
        if (fCurrentSample != nullptr)
            fCurrentSample->percussiveBoost = value;
        renderSample();
        break;
    case kAmpAttack:
        ampEnvGen.setAttack(value);
        renderSample();
        break;
    case kAmpDecay:
        ampEnvGen.setDecay(value);
        renderSample();
        break;
    case kAmpSustain:
        ampEnvGen.setSustain(value);
        renderSample();
        break;
    case kAmpRelease:
        ampEnvGen.setRelease(value);
        renderSample();
        break;
    case kSustainLength:
        if (fCurrentSample != nullptr)
            fCurrentSample->sustainLength = value;
        renderSample();
        break;
    case kFilterCutoff:
        if (fCurrentSample != nullptr)
            fCurrentSample->filterCutoff = value;
        renderSample();
        break;
    case kFilterResonance:
        if (fCurrentSample != nullptr)
            fCurrentSample->filterResonance = value;
        renderSample();
        break;
    case kFilterType:
        if (fCurrentSample != nullptr)
            fCurrentSample->filterType = (Filter::FilterType)value;
        renderSample();
        break;
    case kSlot1MidiNumber:
    case kSlot2MidiNumber:
    case kSlot3MidiNumber:
    case kSlot4MidiNumber:
    case kSlot5MidiNumber:
    case kSlot6MidiNumber:
    case kSlot7MidiNumber:
    case kSlot8MidiNumber:
        slot = index - kSlot1MidiNumber;
        samplePlayers[slot].midi = (int)value;
        break;
    default:
        std::cerr << "Unknown parameter index: " << index << "  " << value << std::endl;
        break;
    }
}

void WAIVESampler::setState(const char *key, const char *value)
{
    if (strcmp(key, "importSource") == 0)
    {
        std::stringstream filelist(value);
        std::string filename;

        int count = 0;

        while (std::getline(filelist, filename, '|'))
        {
            std::cout << " - " << filename << std::endl;
            sd.addSourceToLibrary(std::string(filename));

            if (count == 0)
            {
                // Todo: load first sample?
            }
            count++;
        }
        fSourceTagString = "";
    }
    else if (strcmp(key, "importSample") == 0)
    {
        std::stringstream filelist(value);
        std::cout << "Importing files:" << std::endl;
        std::string filename;
        while (std::getline(filelist, filename, '|'))
        {
            std::cout << " - " << filename << std::endl;
            import_queue.push(filename);
        }

        if (importerTask->state() != Poco::Task::TASK_RUNNING)
        {
            importerTask->release();
            importerTask = new ImporterTask(this, &import_queue);
            taskManager.start(importerTask);
        }
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
    std::lock_guard<std::mutex> lock(samplePlayerMtx);

    int bundle_length = 0;

    int midiIndex = 0;
    float y;
    for (uint32_t i = 0; i < numFrames; i++)
    {
        // Parse Midi Messages
        while (midiIndex < midiEventCount && midiEvents[midiIndex].frame == i)
        {
            if (midiEvents[midiIndex].size > MidiEvent::kDataSize)
                continue;

            uint8_t status = midiEvents[midiIndex].data[0];
            uint8_t data1 = midiEvents[midiIndex].data[1];
            uint8_t data2 = midiEvents[midiIndex].data[2];

            uint8_t channel = status & 0xF;
            uint8_t type = status >> 4;

            uint8_t note = data1;
            uint8_t velocity = data2;

            if (type == 0x8 || (type == 0x9 && velocity == 0))
            {
                /* NoteOff */

                // do nothing for now.. set release?
            }
            else if (type == 0x9)
            {
                /* NoteOn */
                for (int j = 0; j < samplePlayers.size(); j++)
                {
                    if (samplePlayers[j].active && samplePlayers[j].midi == note)
                    {
                        samplePlayers[j].state = PlayState::TRIGGERED;
                        samplePlayers[j].velocity = (float)velocity / 128;

                        oscClient.sendMessage("/WAIVE_Sampler/Sample", {samplePlayers[j].sampleInfo->name, samplePlayers[j].sampleInfo->tagString, samplePlayers[j].midi});
                    }
                }
            }

            midiIndex++;
        }

        // Mix sample players outputs
        y = 0.0f;

        for (int j = 0; j < samplePlayers.size(); j++)
        {
            SamplePlayer *sp = &samplePlayers[j];
            if (sp->state == PlayState::STOPPED)
                continue;

            if (sp->state == PlayState::TRIGGERED)
            {
                if (sp->length == 0)
                {
                    sp->state = PlayState::STOPPED;
                    continue;
                }
                else
                {
                    sp->state = PlayState::PLAYING;
                    if (sp->startAt < sp->length)
                        sp->ptr = sp->startAt;
                    else
                        sp->ptr = 0;
                }
            }

            y += sp->waveform->at(sp->ptr) * sp->gain * sp->velocity;

            sp->ptr++;
            if (sp->ptr >= sp->length)
            {
                sp->ptr = sp->startAt;
                sp->state = PlayState::STOPPED;
            }
        }

        outputs[0][i] = y;
        outputs[1][i] = y;
    }
}

void WAIVESampler::clear()
{
    std::cout << "WAIVESampler::clear" << std::endl;
    stopSourcePreview();
    fSourceLoaded = false;
    fSourceLength = 0;
    fSourcePath = "";
    fCurrentSample = nullptr;

    pluginUpdate.notify(this, PluginUpdate::kSourceUpdated);
    pluginUpdate.notify(this, PluginUpdate::kSampleLoaded);
    pluginUpdate.notify(this, PluginUpdate::kSampleUpdated);
}

void WAIVESampler::loadSource(const char *fp)
{
    // std::cout << "WAIVESampler::loadSource" << std::endl;
    stopSourcePreview();
    fSourceLoaded = false;
    fSourcePath = std::string(fp);
    converterManager.cancelAll();
    converterManager.joinAll();
    pluginUpdate.notify(this, PluginUpdate::kSourceLoading);

    auto waveformLoad = std::make_unique<WaveformLoaderTask>(tempBuffer, &tempBufferMutex, std::string(fp), sampleRate);
    std::cout << "WaveformLoaderTask created" << std::endl;
    taskManager.start(waveformLoad.get());
    waveformLoad.release();
}

void WAIVESampler::newSample()
{
    if (fCurrentSample != nullptr)
    {
        // duplicating..
        std::shared_ptr<SampleInfo> s = sd.duplicateSampleInfo(fCurrentSample);
        s->adsr = ADSR_Params(ampEnvGen.getADSR());
        s->saved = false;
        s->waive = true;

        fCurrentSample = s;
        renderSample();
    }
    else
    {
        setParameterValue(kSampleVolume, 1.0f);
        setParameterValue(kSamplePitch, 1.0f);
        setParameterValue(kAmpAttack, 0.0f);
        setParameterValue(kAmpDecay, 50.f);
        setParameterValue(kAmpSustain, 0.7f);
        setParameterValue(kAmpRelease, 100.f);
        setParameterValue(kSustainLength, 100.f);
        setParameterValue(kPercussiveBoost, 0.3f);
        setParameterValue(kFilterCutoff, 0.999f);
        setParameterValue(kFilterResonance, 0.0f);
        setParameterValue(kFilterType, 0.0);

        std::shared_ptr<SampleInfo> s(new SampleInfo(-1, sd.getNewSampleName("new_sample.wav"), "", true));
        s->adsr = ADSR_Params(ampEnvGen.getADSR());
        s->saved = false;
        fCurrentSample = s;
    }

    pluginUpdate.notify(this, PluginUpdate::kSampleLoaded);
    pluginUpdate.notify(this, PluginUpdate::kParametersChanged);
}

void WAIVESampler::addCurrentSampleToLibrary()
{
    if (fCurrentSample == nullptr || !fSampleLoaded)
        return;

    auto embedding = getEmbedding(editorPreviewWaveform);
    fCurrentSample->embedX = embedding.first;
    fCurrentSample->embedY = embedding.second;

    fCurrentSample->adsr = ADSR_Params(ampEnvGen.getADSR());

    saveWaveform(sd.getFullSamplePath(fCurrentSample).c_str(), &(editorPreviewWaveform->at(0)), fCurrentSample->sampleLength, sampleRate);

    if (fCurrentSample->saved)
        sd.updateSample(fCurrentSample);
    else
    {
        sd.addToLibrary(fCurrentSample);
        // add it to next avaliable sample player
        for (int i = 0; i < samplePlayers.size(); i++)
        {
            if (!samplePlayers[i].active)
            {
                loadSamplePlayer(fCurrentSample, samplePlayers[i], samplePlayerWaveforms[i]);
                break;
            }
        }
    }

    mapPreviewPlayer->sampleInfo = nullptr;
}

void WAIVESampler::loadSamplePreview(int id)
{
    if (mapPreviewPlayer->sampleInfo == nullptr || mapPreviewPlayer->sampleInfo->getId() != id)
        loadSlot(9, id);

    if (id >= 0 && mapPreviewPlayer->state == PlayState::STOPPED)
        mapPreviewPlayer->state = PlayState::TRIGGERED;
}

void WAIVESampler::loadSourcePreview(const std::string &fp)
{
    stopSourcePreview();
    std::lock_guard<std::mutex> lock(samplePlayerMtx);

    int size = loadWaveform(fp.c_str(), *sourcePreviewWaveform, sampleRate);
    if (size == 0)
    {
        std::cout << fp << " not avaliable to load...\n";
        return;
    }
    sourcePreviewPlayer->waveform = sourcePreviewWaveform;
    sourcePreviewPlayer->startAt = 0;
    sourcePreviewPlayer->length = size;
    sourcePreviewPlayer->active = true;
    sourcePreviewPlayer->state = PlayState::TRIGGERED;
}

void WAIVESampler::playSourcePreview()
{
    if (!fSourceLoaded || fSourceWaveform.size() == 0)
        return;

    std::lock_guard<std::mutex> lock(samplePlayerMtx);
    if (sourcePreviewPlayer->state == PlayState::PLAYING)
    {
        sourcePreviewPlayer->state = PlayState::STOPPED;
    }
    else
    {
        sourcePreviewPlayer->startAt = fCurrentSample->sourceStart;
        sourcePreviewPlayer->length = fSourceLength;
        sourcePreviewPlayer->waveform = &fSourceWaveform;
        sourcePreviewPlayer->active = true;
        sourcePreviewPlayer->state = PlayState::TRIGGERED;
    }
}

void WAIVESampler::stopSourcePreview()
{
    std::lock_guard<std::mutex> lock(samplePlayerMtx);
    sourcePreviewPlayer->state = PlayState::STOPPED;
    sourcePreviewPlayer->active = false;
    sourcePreviewPlayer->ptr = 0;
}

void WAIVESampler::loadSample(int id)
{
    loadSample(sd.findSample(id));
}

void WAIVESampler::loadSample(std::shared_ptr<SampleInfo> s)
{
    if (s == nullptr)
    {
        fCurrentSample = nullptr;
        pluginUpdate.notify(this, PluginUpdate::kSampleCleared);
        pluginUpdate.notify(this, PluginUpdate::kParametersChanged);

        return;
    }

    bool newSource = true;
    if (fCurrentSample != nullptr && fCurrentSample->source.compare(s->source) == 0)
        newSource = false;

    fCurrentSample = s;
    fSampleLoaded = false;

    if (newSource)
        loadSource(fCurrentSample->source.c_str());
    setParameterValue(kSampleVolume, fCurrentSample->volume);
    setParameterValue(kSamplePitch, fCurrentSample->pitch);
    setParameterValue(kAmpAttack, fCurrentSample->adsr.attack);
    setParameterValue(kAmpDecay, fCurrentSample->adsr.decay);
    setParameterValue(kAmpSustain, fCurrentSample->adsr.sustain);
    setParameterValue(kAmpRelease, fCurrentSample->adsr.release);
    setParameterValue(kSustainLength, fCurrentSample->sustainLength);
    setParameterValue(kPercussiveBoost, fCurrentSample->percussiveBoost);
    setParameterValue(kFilterCutoff, fCurrentSample->filterCutoff);
    setParameterValue(kFilterResonance, fCurrentSample->filterResonance);
    setParameterValue(kFilterType, fCurrentSample->filterType);
    selectWaveform(&fSourceWaveform, fCurrentSample->sourceStart);

    fCurrentSample->sampleLength = loadWaveform(sd.getSamplePath(fCurrentSample).c_str(), *editorPreviewWaveform, sampleRate);

    fSampleLoaded = true;
    pluginUpdate.notify(this, PluginUpdate::kSampleLoaded);

    renderSample();

    pluginUpdate.notify(this, PluginUpdate::kParametersChanged);
}

void WAIVESampler::loadPreset(Preset preset)
{
    setParameterValue(kSampleVolume, preset.volume);
    setParameterValue(kSamplePitch, preset.pitch);
    setParameterValue(kAmpAttack, preset.adsr.attack);
    setParameterValue(kAmpDecay, preset.adsr.decay);
    setParameterValue(kAmpSustain, preset.adsr.sustain);
    setParameterValue(kAmpRelease, preset.adsr.release);
    setParameterValue(kSustainLength, preset.sustainLength);
    setParameterValue(kPercussiveBoost, preset.percussiveBoost);
    setParameterValue(kFilterCutoff, preset.filterCutoff);
    setParameterValue(kFilterResonance, preset.filterResonance);
    setParameterValue(kFilterType, preset.filterType);

    fSampleLoaded = true;
    pluginUpdate.notify(this, PluginUpdate::kSampleLoaded);

    renderSample();
    pluginUpdate.notify(this, PluginUpdate::kParametersChanged);
}

void WAIVESampler::selectWaveform(std::vector<float> *source, int start)
{
    if (!fSourceLoaded)
        return;

    if (start >= source->size())
        start = 0;

    fSampleLoaded = false;
    if (fCurrentSample == nullptr)
        newSample();

    fCurrentSample->sourceStart = start;
    fSampleLoaded = true;

    renderSample();
    triggerPreview();
    pluginUpdate.notify(this, PluginUpdate::kSampleUpdated);
}

void WAIVESampler::renderSample()
{
    // LOG_LOCATION
    if (!fSampleLoaded || fCurrentSample == nullptr || !fSourceLoaded)
        return;

    std::lock_guard<std::mutex> lock(samplePlayerMtx);

    fCurrentSample->sampleLength = ampEnvGen.getLength(fCurrentSample->sustainLength);
    fCurrentSample->sampleLength = std::min(fCurrentSample->sampleLength, fSourceLength - fCurrentSample->sourceStart);

    auto minmax = std::minmax_element(
        &fSourceWaveform[fCurrentSample->sourceStart],
        &fSourceWaveform[fCurrentSample->sourceStart + fCurrentSample->sampleLength]);
    float normaliseRatio = std::max(-(*minmax.first), *minmax.second);
    if (std::abs(normaliseRatio) <= 0.0001f)
        normaliseRatio = 1.0f;

    if (editorPreviewWaveform->size() < fCurrentSample->sampleLength)
        editorPreviewWaveform->resize(fCurrentSample->sampleLength);

    ampEnvGen.reset();
    ampEnvGen.trigger();

    float amp = ampEnvGen.getValue();

    // filter setup
    sampleFilter.setCutoff(fCurrentSample->filterCutoff);
    sampleFilter.setResonance(fCurrentSample->filterResonance);
    sampleFilter.mode = fCurrentSample->filterType;
    sampleFilter.reset();

    // delta (pitch) envelope
    float deltaStart = fCurrentSample->pitch + fCurrentSample->percussiveBoost * 3.0f;
    float delta = deltaStart;
    int deltaEnvLength = sampleRate / 20;
    if (d_isZero(fCurrentSample->percussiveBoost))
        deltaEnvLength = -1;

    float y = 0.0f;
    int index = 0;
    float indexF = 0.0f;

    for (int i = 0; i < fCurrentSample->sampleLength; i++)
    {
        index = (int)indexF;
        if (fCurrentSample->sourceStart + index >= fSourceLength)
        {
            fCurrentSample->sampleLength = i;
            editorPreviewPlayer->length = i;
            break;
        }

        if (i < deltaEnvLength)
            delta = interpolate((float)i / (float)deltaEnvLength, deltaStart, fCurrentSample->pitch, 1.0f, true);
        else
            delta = fCurrentSample->pitch;

        y = fSourceWaveform[fCurrentSample->sourceStart + index];
        y = sampleFilter.process(y);

        editorPreviewWaveform->at(i) = std::clamp(y * fCurrentSample->volume * amp / normaliseRatio, -1.0f, 1.0f);

        indexF += delta;

        if (1000.f * indexF / sampleRate >= ampEnvGen.getAttack() + ampEnvGen.getDecay() + fCurrentSample->sustainLength && ampEnvGen.getStage() != ADSR_Stage::RELEASE)
            ampEnvGen.release();
        ampEnvGen.process();

        if (!ampEnvGen.active)
        {
            fCurrentSample->sampleLength = i;
            editorPreviewPlayer->length = i;
            break;
        }

        amp = ampEnvGen.getValue();
    }

    editorPreviewPlayer->length = fCurrentSample->sampleLength;
    editorPreviewPlayer->ptr = 0;
    editorPreviewPlayer->active = true;

    fSampleLoaded = true;
    pluginUpdate.notify(this, PluginUpdate::kSampleUpdated);
}

void WAIVESampler::loadSamplePlayer(std::shared_ptr<SampleInfo> info, SamplePlayer &sp, std::vector<float> &buffer)
{
    LOG_LOCATION
    if (info == nullptr)
    {
        sp.active = false;
        return;
    }

    std::lock_guard<std::mutex> lock(samplePlayerMtx);

    sp.state = PlayState::STOPPED;
    sp.ptr = 0;
    int length = loadWaveform(sd.getFullSamplePath(info).c_str(), buffer, sampleRate);

    if (length == 0)
    {
        sp.active = false;
        std::cerr << "WAIVESampler::loadSamplePlayer: Sample waveform length 0" << std::endl;
        return;
    }

    sp.length = length;
    sp.active = true;
    sp.sampleInfo = info;
    sp.sampleInfo->tagString.assign(info->tagString); // no idea why we must do this only for tagString...

    pluginUpdate.notify(this, PluginUpdate::kSlotLoaded);
}

void WAIVESampler::clearSamplePlayer(SamplePlayer &sp)
{
    std::cout << "WAIVESampler::clearSamplePlayer\n";
    std::lock_guard<std::mutex> lock(samplePlayerMtx);
    sp.state = PlayState::STOPPED;
    sp.active = false;
    sp.sampleInfo = nullptr;
    sp.length = 0;
    pluginUpdate.notify(this, PluginUpdate::kSlotLoaded);
}

void WAIVESampler::loadSlot(int slot, int id)
{
    std::shared_ptr<SampleInfo> info = sd.findSample(id);
    loadSamplePlayer(info, samplePlayers.at(slot), samplePlayerWaveforms.at(slot));
}

void WAIVESampler::triggerPreview()
{
    if (!fSampleLoaded)
        return;

    if (samplePlayers[8].state == PlayState::STOPPED)
    {
        samplePlayers[8].state = PlayState::TRIGGERED;
        samplePlayers[8].active = true;
    }
}

std::pair<float, float> WAIVESampler::getEmbedding(std::vector<float> *wf)
{
    getFeatures(wf, &mTSNEInput);

    const char *inputNamesCstrs[] = {mTSNEInputNames[0].c_str()};
    const char *outputNamesCstrs[] = {mTSNEOutputNames[0].c_str()};

    mTSNE->Run(
        mRunOptions,
        inputNamesCstrs,
        mTSNEInputTensor.data(),
        mTSNEInputTensor.size(),
        outputNamesCstrs,
        mTSNEOutputTensor.data(),
        mTSNEOutputTensor.size());

    float embedX = mTSNEOutput[0];
    float embedY = mTSNEOutput[1];
    return std::pair<float, float>(embedX, embedY);
}

void WAIVESampler::getFeatures(std::vector<float> *wf, std::vector<float> *feature)
{
    int length = 64;

    float epsilon = 0.000000001f;
    float X_mean = -13.617876;
    float X_std = 7.084826;

    std::vector<std::vector<float>> melspec = fe.getMelSpectrogram(wf);

    std::fill(feature->begin(), feature->end(), (std::log(epsilon) - X_mean) / X_std);
    int width = std::min((int)melspec.size(), length);

    // printf("n_mel: %d width: %d length: %d feature->size: %d \n", n_mel, width, length, feature->size());

    // std::ofstream melofs("melspec.csv", std::ofstream::trunc);
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < melspec[i].size(); j++)
        {
            float val = std::log(epsilon + melspec[i][j]);
            feature->at(j * length + i) = (val - X_mean) / X_std;
            // melofs << val;
            // if (j < melspec[i].size() - 1)
            //     melofs << " ";
        }
        // if (i < width - 1)
        //     melofs << "\n";
    }
    // melofs.close();
}

void WAIVESampler::sampleRateChanged(double newSampleRate)
{
    sampleRate = newSampleRate;
    ampEnvGen.sampleRate = newSampleRate;
}

void WAIVESampler::onTaskFinished(Poco::TaskFinishedNotification *pNf)
{
    Poco::Task *pTask = pNf->task();
    if (pTask == nullptr)
    {
        std::cerr << "Error: Task is null" << std::endl;
        return;
    }
    std::cout << "WAIVESampler::onTaskFinished: " << pTask->name() << std::endl;

    const std::string taskName = pTask->name();

    if (taskName == "WaveformLoaderTask")
    {
        if (pTask->isCancelled())
        {
            fCurrentSample->source = "";
            fSourcePath = "";
            fSourceLength = 0;
            pNf->release();
            return;
        }

        std::lock_guard<std::mutex> lock(tempBufferMutex);

        fSourceLength = tempBuffer->size();
        fSourceWaveform.resize(fSourceLength);

        std::copy(tempBuffer->begin(), tempBuffer->begin() + fSourceLength, fSourceWaveform.begin());

        if (!fCurrentSample)
            newSample();

        if (fSourceLength > 0)
        {
            fSourceLoaded = true;
            fCurrentSample->source = fSourcePath;
            fCurrentSample->tagString = fSourceTagString;
            selectWaveform(&fSourceWaveform, fCurrentSample->sourceStart);

            taskManager.start(new FeatureExtractorTask(this));
            pluginUpdate.notify(this, PluginUpdate::kSourceLoaded);
        }
        else
        {
            std::cerr << "Source failed to load" << std::endl;
            fCurrentSample->source = "";
            fCurrentSample->tagString = "";
            fSourcePath = "";
        }
    }

    pNf->release();
}

void WAIVESampler::onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg)
{
    switch (arg)
    {
    case SampleDatabase::DatabaseUpdate::SOURCE_PREVIEW_READY:
        loadSourcePreview(sd.getSourcePreview());
        break;

    default:
        break;
    }
}

Plugin *createPlugin()
{
    return new WAIVESampler();
}

int loadWaveform(const char *fp, std::vector<float> &buffer, int sampleRate, int flags)
{
    // TODO: load on another thread!

    // printf("WAIVESampler::loadWaveform %s\n", fp);

    SndfileHandle fileHandle(fp, 16, flags);
    int sampleLength = fileHandle.frames();

    if (sampleLength == 0)
    {
        std::cerr << "Error: Unable to open input file " << fp << std::endl;
        return 0;
    }

    int sampleChannels = fileHandle.channels();
    int fileSampleRate = fileHandle.samplerate();

    // std::cout << "sampleChannels: " << sampleChannels << " "
    //           << " sampleLength: " << sampleLength
    //           << " fileSampleRate: " << fileSampleRate
    //           << " (sampleRate: " << sampleRate << ")\n";

    std::vector<float> sample;
    sample.resize(sampleLength * sampleChannels);
    fileHandle.read(&sample.at(0), sampleLength * sampleChannels);

    std::vector<float> sample_tmp;

    // resample data
    if (fileSampleRate != sampleRate)
    {
        int new_size = (int)((double)sampleLength * sampleRate / fileSampleRate + 1);
        sample_tmp.resize(static_cast<size_t>(new_size * sampleChannels));

        SRC_DATA src_data;
        src_data.input_frames = sampleLength;
        src_data.data_out = sample_tmp.data(); //&sample_tmp.at(0);
        src_data.data_in = sample.data();
        src_data.output_frames = new_size;
        src_data.src_ratio = (float)sampleRate / fileSampleRate;

        std::cout << "RESAMPLING WAVEFORM:\n";
        std::cout << "  fileSampleRate: " << fileSampleRate << std::endl;
        std::cout << "      sampleRate: " << sampleRate << std::endl;
        std::cout << " sample_channels: " << sampleChannels << std::endl;
        std::cout << "    input_frames: " << src_data.input_frames << std::endl;
        std::cout << "   output_frames: " << src_data.output_frames << std::endl;
        std::cout << "       src_ratio: " << src_data.src_ratio << std::endl;

        int result = src_simple(&src_data, SRC_SINC_BEST_QUALITY, sampleChannels);
        if (result != 0)
            std::cerr << "Failed to convert sample rate: " << src_strerror(result) << std::endl;
        else
            std::cout << "sample rate sucessfully converted\n";
    }
    else
    {
        sample_tmp = sample;
    }

    if (buffer.size() < sampleLength)
        buffer.resize(sampleLength + 1); // TODO: this sometimes reallocates the pointer!

    //  TODO: mix to Mono before sample rate conversion??
    // printf("buffer.size() %d, sample_tmp.size() %d\n", buffer.size(), sample_tmp.size());
    if (sampleChannels > 1)
    {
        for (int i = 0; i < sampleLength; i++)
            buffer[i] = (sample_tmp[i * sampleChannels] + sample_tmp[i * sampleChannels + 1]) * 0.5f;
    }
    else
    {
        for (int i = 0; i < sampleLength; i++)
            buffer[i] = sample_tmp[i];
    }

    // std::cout << "loadWaveform sampleLength: " << sampleLength << std::endl;
    return sampleLength;
}

bool saveWaveform(const char *fp, const float *buffer, sf_count_t size, int sampleRate)
{
    std::cout << "WAIVESampler::saveWaveform to " << fp << std::endl;

    SndfileHandle file = SndfileHandle(
        fp,
        SFM_WRITE,
        SF_FORMAT_WAV | SF_FORMAT_PCM_16,
        1,
        sampleRate);

    if (file.error())
    {
        std::cerr << "Error: Unable to open output file " << fp << " for writing\n  error: " << sf_error_number(file.error()) << std::endl;
        return false;
    }

    sf_count_t writtenFrames = file.write(buffer, size);
    if (writtenFrames != size)
    {
        std::cerr << "Error: Failed to write all frames to " << fp << std::endl;
        return false;
    }

    std::cout << "saveWaveform() done" << std::endl;

    return true;
}

END_NAMESPACE_DISTRHO