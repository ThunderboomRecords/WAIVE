#include "WAIVESampler.hpp"

START_NAMESPACE_DISTRHO

uint8_t defaultMidiMap[] = {36, 38, 47, 50, 43, 42, 46, 51, 49};

ImporterTask::ImporterTask(WAIVESampler *ws, ThreadsafeQueue<std::string> *queue) : Poco::Task("ImporterTask"), _ws(ws), _queue(queue){};

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
    int id = rand() % 10000;

    size_t pos = fp.find_last_of("/\\");

    std::string name, folder;

    if (pos == std::string::npos)
    {
        name.assign(fp);
        folder.assign("./");
    }
    else
    {
        name.assign(fp.substr(pos + 1));
        folder.assign(fp.substr(0, pos + 1));
    }

    std::cout << "         id: " << id << std::endl;
    std::cout << "     folder: " << folder << std::endl;
    std::cout << "       name: " << name << std::endl;

    std::vector<float> sampleCopy;
    int sampleLength = loadWaveform(fp.c_str(), &sampleCopy, _ws->getSampleRate());

    if (sampleLength == 0)
    {
        std::cout << "sampleLength is 0\n";
        return;
    }

    std::shared_ptr<SampleInfo> info(new SampleInfo(id, name, _ws->sd.getSampleFolder(), false));
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
    info->tags.push_back({"imported"});

    // TODO: render loaded sample...
    auto embedding = _ws->getEmbedding(&sampleCopy);
    info->embedX = embedding.first;
    info->embedY = embedding.second;

    _ws->sd.addToLibrary(info);

    saveWaveform(_ws->sd.getSamplePath(info).c_str(), &(sampleCopy.at(0)), sampleLength, _ws->getSampleRate());
    std::cout << " - import done\n";
};

FeatureExtractorTask::FeatureExtractorTask(WAIVESampler *ws) : Poco::Task("FeatureExtractorTask"), _ws(ws) {}

void FeatureExtractorTask::runTask()
{
    if (_ws->fSourceLength == 0)
        return;

    int frame = 0;
    int length = _ws->fSourceLength;
    Gist<float> gist(512, (int)_ws->getSampleRate());
    _ws->fSourceFeatures.clear();

    float pStep = (float)gist.getAudioFrameSize() / length;
    float p = 0.0f;

    while (frame < length - gist.getAudioFrameSize() && !isCancelled())
    {
        gist.processAudioFrame(std::vector<float>(_ws->fSourceWaveform.begin() + frame, _ws->fSourceWaveform.begin() + frame + gist.getAudioFrameSize()));
        float onset = gist.spectralDifferenceHWR();
        if (onset > 150.0f)
        {
            // TODO: use mutex to reserve vector operation
            _ws->fSourceFeatures.push_back({FeatureType::Onset,
                                            "onset",
                                            onset,
                                            frame,
                                            frame});
        }
        p += pStep;
        setProgress(p);

        frame += gist.getAudioFrameSize();
    }

    setProgress(1.0f);
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
                               httpClient(&taskManager),
                               sd(&httpClient),
                               fe(getSampleRate(), 1024, 441, 64, WindowType::HanningWindow),
                               importerTask(new ImporterTask(this, &import_queue)),
                               sourceFeatureTask(new FeatureExtractorTask(this))
{
    if (isDummyInstance())
    {
        std::cout << "** dummy instance" << std::endl;
        return;
    }

    printf(" VERSION: %d.%d.%d\n", V_MAJ, V_MIN, V_PAT);

    srand(time(NULL));

    // Register notifications
    taskManager.addObserver(Poco::Observer<WAIVESampler, Poco::TaskFinishedNotification>(*this, &WAIVESampler::onTaskFinished));

    samplePlayerWaveforms.resize(10);
    for (int i = 0; i < 10; i++)
    {
        SamplePlayer sp;
        sp.waveform = &samplePlayerWaveforms[i];
        samplePlayers.push_back(sp);
    }

    editorPreviewPlayer = &samplePlayers[8];
    editorPreviewWaveform = &samplePlayerWaveforms[8];
    mapPreviewPlayer = &samplePlayers[9];
    mapPreviewWaveform = &samplePlayerWaveforms[9];

    for (int i = 0; i < 8; i++)
        samplePlayers[i].midi = defaultMidiMap[i];

    // Load models
    std::cout << "Loading TSNE model...\n";
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

        PrintModelDetails("TSNE model", mTSNEInputNames, mTSNEOutputNames, mTSNEInputShape, mTSNEOutputShape);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    // OSC Test
    oscClient.sendMessage("/WAIVE_Sampler", {"WAIVESampler started"});
}

WAIVESampler::~WAIVESampler()
{
    std::cout << "closing WAIVESampler..." << std::endl;

    taskManager.cancelAll();
    taskManager.joinAll();
}

void WAIVESampler::initParameter(uint32_t index, Parameter &parameter)
{
    int slot = 0;
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
        parameter.hints = kParameterIsInteger;
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
    if (strcmp(key, "filename") == 0)
    {
        loadSource(value);
    }
    else if (strcmp(key, "import") == 0)
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
    const float **, // incoming audio        sourceFeatureTask = new FeatureExtractorTask(this);

    float **outputs,             // outgoing audio
    uint32_t numFrames,          // size of block to process
    const MidiEvent *midiEvents, // MIDI pointer
    uint32_t midiEventCount      // Number of MIDI events in block
)
{
    samplePlayerMtx.lock();

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

                        oscClient.sendMessage("/WAIVE_Sampler", {samplePlayers[j].sampleInfo->name, samplePlayers[j].midi});
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
                    sp->ptr = 0;
                }
            }

            y += sp->waveform->at(sp->ptr) * sp->gain * sp->velocity;

            sp->ptr++;
            if (sp->ptr >= sp->length)
            {
                sp->ptr = 0;
                sp->state = PlayState::STOPPED;
            }
        }

        outputs[0][i] = y;
        outputs[1][i] = y;
    }

    samplePlayerMtx.unlock();
}

void WAIVESampler::loadSource(const char *fp)
{
    std::cout << "WAIVESampler::loadSource" << std::endl;

    if (sourceFeatureTask->state() != Poco::Task::TASK_FINISHED)
    {
        sourceFeatureTask->cancel();
        while (sourceFeatureTask->state() == Poco::Task::TASK_RUNNING)
        {
        }
    }

    fSourceLoaded = false;
    pluginUpdate.notify(this, PluginUpdate::kSourceLoading);

    fSourceLength = loadWaveform(fp, &fSourceWaveform, sampleRate);

    if (fSourceLength > 0)
    {
        fSourceLoaded = true;
        fCurrentSample->source = std::string(fp);
        if (fCurrentSample->sourceStart >= fSourceLength)
            selectWaveform(&fSourceWaveform, 0);

        if (sourceFeatureTask != NULL)
            sourceFeatureTask->release();

        sourceFeatureTask = new FeatureExtractorTask(this);
        taskManager.start(sourceFeatureTask);

        pluginUpdate.notify(this, PluginUpdate::kSourceLoaded);
    }
    else
    {
        std::cerr << "Source failed to load\n";
        fCurrentSample->source = "";
    }
}

void WAIVESampler::newSample()
{
    LOG_LOCATION

    // TODO: save current sample before creating a new one?

    if (fCurrentSample != nullptr)
    {
        // duplicating..
        std::shared_ptr<SampleInfo> s = sd.duplicateSampleInfo(fCurrentSample);
        s->name = fmt::format("{:d}_{}", s->getId(), s->name);
        s->adsr = ADSR_Params(ampEnvGen.getADSR());
        s->saved = false;
        s->waive = true;

        fCurrentSample = s;
        renderSample();
    }
    else
    {
        // new default sample
        fSourceLoaded = false;
        fSampleLoaded = false;
        fSourceLength = 0;
        fSourceWaveform.clear();

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

        time_t current_time = time(NULL);
        std::string name = fmt::format("Sample{:d}.wav", current_time % 10000);
        std::shared_ptr<SampleInfo> s(new SampleInfo(current_time, name, sd.getSampleFolder(), true));
        s->adsr = ADSR_Params(ampEnvGen.getADSR());
        s->saved = false;
        fCurrentSample = s;
    }

    std::cout << fCurrentSample->getId() << std::endl;

    pluginUpdate.notify(this, PluginUpdate::kSourceLoaded);
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

    sd.addToLibrary(fCurrentSample);
    saveWaveform(sd.getSamplePath(fCurrentSample).c_str(), &(editorPreviewWaveform->at(0)), fCurrentSample->sampleLength, sampleRate);
}

void WAIVESampler::loadPreview(int id)
{
    if (samplePlayers[9].sampleInfo == nullptr || samplePlayers[9].sampleInfo->getId() != id)
        loadSlot(9, id);

    if (id >= 0 && mapPreviewPlayer->state == PlayState::STOPPED)
        mapPreviewPlayer->state = PlayState::TRIGGERED;
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

    fCurrentSample->sampleLength = loadWaveform(sd.getSamplePath(fCurrentSample).c_str(), editorPreviewWaveform, sampleRate);

    fSampleLoaded = true;
    pluginUpdate.notify(this, PluginUpdate::kSampleLoaded);

    renderSample();

    pluginUpdate.notify(this, PluginUpdate::kParametersChanged);
}

void WAIVESampler::selectWaveform(std::vector<float> *source, int start)
{
    if (!fSourceLoaded)
        return;

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

    samplePlayerMtx.lock();

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
    samplePlayerMtx.unlock();
    pluginUpdate.notify(this, PluginUpdate::kSampleUpdated);
}

void WAIVESampler::loadSamplePlayer(std::shared_ptr<SampleInfo> info, SamplePlayer &sp, std::vector<float> &buffer)
{
    // LOG_LOCATION
    if (info == nullptr)
    {
        sp.active = false;
        return;
    }

    samplePlayerMtx.lock();

    sp.state = PlayState::STOPPED;
    sp.ptr = 0;
    int length = loadWaveform(sd.getSamplePath(info).c_str(), &buffer, sampleRate);

    if (length == 0)
    {
        sp.active = false;
        samplePlayerMtx.unlock();
        std::cerr << "WAIVESampler::loadSamplePlayer: Sample waveform length 0" << std::endl;
        return;
    }

    sp.length = length;
    sp.active = true;
    sp.sampleInfo = info;

    pluginUpdate.notify(this, PluginUpdate::kSlotLoaded);
    samplePlayerMtx.unlock();
}

void WAIVESampler::clearSamplePlayer(SamplePlayer &sp)
{
    std::cout << "WAIVESampler::clearSamplePlayer\n";
    samplePlayerMtx.lock();
    sp.state = PlayState::STOPPED;
    sp.active = false;
    sp.sampleInfo = nullptr;
    sp.length = 0;
    pluginUpdate.notify(this, PluginUpdate::kSlotLoaded);
    samplePlayerMtx.unlock();
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
    std::cout << "WAIVESampler::onTaskFinished: " << pTask->name() << std::endl;
    pTask->release();
}

Plugin *createPlugin()
{
    return new WAIVESampler();
}

int loadWaveform(const char *fp, std::vector<float> *buffer, int sampleRate)
{
    // printf("WAIVESampler::loadWaveform %s\n", fp);

    SndfileHandle fileHandle(fp);
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
        int new_size = sampleChannels * (int)((float)sampleLength * sampleRate / fileSampleRate + 1);
        sample_tmp.resize(new_size);

        SRC_DATA src_data;
        src_data.input_frames = sampleLength;
        src_data.data_out = &sample_tmp.at(0);
        src_data.data_in = &sample.at(0);
        src_data.output_frames = new_size;
        src_data.src_ratio = (float)sampleRate / fileSampleRate;

        // std::cout << "RESAMPLING WAVEFORM:\n";
        // std::cout << " sample_channels: " << sampleChannels << std::endl;
        // std::cout << "    input_frames: " << sampleLength << std::endl;
        // std::cout << "   output_frames: " << new_size << std::endl;
        // std::cout << "       src_ratio: " << src_data.src_ratio << std::endl;

        int result = src_simple(&src_data, SRC_SINC_BEST_QUALITY, sampleChannels);
        if (result != 0)
            std::cerr << "Failed to convert sample rate: " << src_strerror(result) << std::endl;
    }
    else
    {
        sample_tmp = sample;
    }

    if (buffer->size() < sampleLength)
        buffer->resize(sampleLength);

    if (sampleChannels > 1)
    {
        for (int i = 0; i < sampleLength; i++)
            buffer->operator[](i) = (sample_tmp[i * sampleChannels] + sample_tmp[i * sampleChannels + 1]) * 0.5f;
    }
    else
    {
        for (int i = 0; i < sampleLength; i++)
            buffer->operator[](i) = sample_tmp[i];
    }

    return sampleLength;
}

bool saveWaveform(const char *fp, float *buffer, sf_count_t size, int sampleRate)
{
    // std::cout << "WAIVESampler::saveWaveform" << std::endl;

    SndfileHandle file = SndfileHandle(
        fp,
        SFM_WRITE,
        SF_FORMAT_WAV | SF_FORMAT_PCM_16,
        1,
        sampleRate);

    file.write(buffer, size);

    return true;
}

END_NAMESPACE_DISTRHO