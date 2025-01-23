#include "WAIVESampler.hpp"
#include "Tasks.hpp"

START_NAMESPACE_DISTRHO

uint8_t defaultMidiMap[] = {36, 38, 47, 50, 43, 42, 46, 51, 49};

WAIVESampler::WAIVESampler() : Plugin(kParameterCount, 0, 0),
                               sampleRate(getSampleRate()),
                               fNormalisationRatio(1.0f),
                               fCurrentSample(nullptr),
                               ampEnvGen(getSampleRate(), ENV_TYPE::ADSR, {10, 50, 0.7, 100}),
                               oscClient("localhost", 8000),
                               taskManager("plugin manager", 1, 8, 60, 0),
                               httpClient(&taskManager),
                               sd(&httpClient),
                               fe(getSampleRate(), 1024, 441, 64, WindowType::HanningWindow),
                               importerTask(new ImporterTask(this, &import_queue))
{
    if (isDummyInstance())
    {
        std::cout << "** dummy instance" << std::endl;
        // return;
    }

    printf(" VERSION: %d.%d.%d\n", V_MAJ, V_MIN, V_PAT);

    // Register notifications
    taskManager.addObserver(Poco::Observer<WAIVESampler, Poco::TaskFinishedNotification>(*this, &WAIVESampler::onTaskFinished));
    taskManager.addObserver(Poco::Observer<WAIVESampler, Poco::TaskFailedNotification>(*this, &WAIVESampler::onTaskFailed));
    sd.databaseUpdate += Poco::delegate(this, &WAIVESampler::onDatabaseChanged);
    sd.taskManager.addObserver(Poco::Observer<WAIVESampler, Poco::TaskFinishedNotification>(*this, &WAIVESampler::onTaskFinished));

    samplePlayerWaveforms.resize(NUM_SLOTS + 3);
    for (int i = 0; i < NUM_SLOTS + 3; i++)
    {
        SamplePlayer sp;
        sp.waveform = &samplePlayerWaveforms[i];
        samplePlayers.push_back(sp);
    }

    editorPreviewPlayer = &samplePlayers[NUM_SLOTS];
    editorPreviewWaveform = &samplePlayerWaveforms[NUM_SLOTS];
    mapPreviewPlayer = &samplePlayers[NUM_SLOTS + 1];
    mapPreviewWaveform = &samplePlayerWaveforms[NUM_SLOTS + 1];
    sourcePreviewPlayer = &samplePlayers[NUM_SLOTS + 2];
    sourcePreviewWaveform = &samplePlayerWaveforms[NUM_SLOTS + 2];

    for (int i = 0; i < NUM_SLOTS; i++)
        samplePlayers[i].midi = defaultMidiMap[i % 9];

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

    taskManager.cancelAll();
    taskManager.joinAll();

    std::cout << " - WAIVESampler::~WAIVESampler() DONE" << std::endl;
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
        parameter.ranges.max = 5000.0f;
        parameter.ranges.def = 200.0f;
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
    case kSlot9MidiNumber:
    case kSlot10MidiNumber:
    case kSlot11MidiNumber:
    case kSlot12MidiNumber:
    case kSlot13MidiNumber:
    case kSlot14MidiNumber:
    case kSlot15MidiNumber:
    case kSlot16MidiNumber:
    case kSlot17MidiNumber:
    case kSlot18MidiNumber:
        slot = index - kSlot1MidiNumber;
        parameter.name = fmt::format("Sample {:d} Midi Number", slot + 1).c_str();
        parameter.symbol = fmt::format("Sample{:d}Midi", slot + 1).c_str();
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 127.0f;
        parameter.ranges.def = (float)defaultMidiMap[slot % 9];
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
    case kSlot9MidiNumber:
    case kSlot10MidiNumber:
    case kSlot11MidiNumber:
    case kSlot12MidiNumber:
    case kSlot13MidiNumber:
    case kSlot14MidiNumber:
    case kSlot15MidiNumber:
    case kSlot16MidiNumber:
    case kSlot17MidiNumber:
    case kSlot18MidiNumber:
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
        break;
    case kSamplePitch:
        if (fCurrentSample != nullptr)
            fCurrentSample->pitch = value;
        break;
    case kPercussiveBoost:
        if (fCurrentSample != nullptr)
            fCurrentSample->percussiveBoost = value;
        break;
    case kAmpAttack:
        ampEnvGen.setAttack(value);
        break;
    case kAmpDecay:
        ampEnvGen.setDecay(value);
        break;
    case kAmpSustain:
        ampEnvGen.setSustain(value);
        break;
    case kAmpRelease:
        ampEnvGen.setRelease(value);
        break;
    case kSustainLength:
        if (fCurrentSample != nullptr)
            fCurrentSample->sustainLength = value;
        break;
    case kFilterCutoff:
        if (fCurrentSample != nullptr)
            fCurrentSample->filterCutoff = value;
        break;
    case kFilterResonance:
        if (fCurrentSample != nullptr)
            fCurrentSample->filterResonance = value;
        break;
    case kFilterType:
        if (fCurrentSample != nullptr)
            fCurrentSample->filterType = (Filter::FilterType)value;
        break;
    case kSlot1MidiNumber:
    case kSlot2MidiNumber:
    case kSlot3MidiNumber:
    case kSlot4MidiNumber:
    case kSlot5MidiNumber:
    case kSlot6MidiNumber:
    case kSlot7MidiNumber:
    case kSlot8MidiNumber:
    case kSlot9MidiNumber:
    case kSlot10MidiNumber:
    case kSlot11MidiNumber:
    case kSlot12MidiNumber:
    case kSlot13MidiNumber:
    case kSlot14MidiNumber:
    case kSlot15MidiNumber:
    case kSlot16MidiNumber:
    case kSlot17MidiNumber:
    case kSlot18MidiNumber:
        slot = index - kSlot1MidiNumber;
        samplePlayers[slot].midi = (int)value;
        break;
    default:
        std::cerr << "Unknown parameter index: " << index << "  " << value << std::endl;
        break;
    }

    renderSample();
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
        // fSourceTagString = "";
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
    fCurrentSample = nullptr;

    pluginUpdate.notify(this, PluginUpdate::kSourceUpdated);
    pluginUpdate.notify(this, PluginUpdate::kSampleLoaded);
    pluginUpdate.notify(this, PluginUpdate::kSampleUpdated);
}

void WAIVESampler::loadSourceFile(const std::string fp, const std::string tagString)
{
    // std::cout << "WAIVESampler::loadSourceFile" << std::endl;
    stopSourcePreview();

    if (fCurrentSample == nullptr)
        newSample();

    fCurrentSample->sourceInfo.fp = std::string(fp);
    fCurrentSample->sourceInfo.sourceLoaded = false;
    fCurrentSample->sourceInfo.tagString = tagString;

    pluginUpdate.notify(this, PluginUpdate::kSourceLoading);

    WaveformLoaderTask *task = new WaveformLoaderTask("loadSourceBuffer", tempBuffer, &tempBufferMutex, std::string(fp), sampleRate);
    taskManager.start(task);
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
        renderSampleLock = true;
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
        renderSampleLock = false;

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
    std::cout << "WAIVESampler::addCurrentSampleToLibrary()" << std::endl;

    if (fCurrentSample == nullptr)
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
                loadSamplePlayer(i, fCurrentSample);
                break;
            }
        }
    }

    mapPreviewPlayer->sampleInfo = nullptr;
}

void WAIVESampler::loadSamplePreview(int id)
{
    if (mapPreviewPlayer->sampleInfo == nullptr || mapPreviewPlayer->sampleInfo->getId() != id)
        loadSlot(NUM_SLOTS + 1, id);

    if (id >= 0 && mapPreviewPlayer->state == PlayState::STOPPED)
        mapPreviewPlayer->state = PlayState::TRIGGERED;
}

void WAIVESampler::loadSourcePreview(const std::string &fp)
{
    WaveformLoaderTask *task = new WaveformLoaderTask("loadSourcePreview", tempBuffer, &tempBufferMutex, fp, sampleRate);
    taskManager.start(task);
}

void WAIVESampler::playSourcePreview()
{
    // if (!fSourceLoaded || fSourceWaveform.size() == 0)
    //     return;

    // std::lock_guard<std::mutex> lock(samplePlayerMtx);
    // if (sourcePreviewPlayer->state == PlayState::PLAYING)
    // {
    //     sourcePreviewPlayer->state = PlayState::STOPPED;
    // }
    // else
    // {
    //     sourcePreviewPlayer->startAt = fCurrentSample->sourceStart;
    //     sourcePreviewPlayer->length = fSourceLength;
    //     sourcePreviewPlayer->waveform = &fSourceWaveform;
    //     sourcePreviewPlayer->active = true;
    //     sourcePreviewPlayer->state = PlayState::TRIGGERED;
    // }
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
        pluginUpdate.notify(this, PluginUpdate::kSampleLoaded);
        pluginUpdate.notify(this, PluginUpdate::kParametersChanged);

        return;
    }

    fCurrentSample = s;

    renderSampleLock = true;
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
    renderSampleLock = false;

    if (fCurrentSample->sourceInfo.length == 0)
        loadSourceFile(fCurrentSample->sourceInfo.fp);
    else
    {
        fCurrentSample->sourceInfo.sourceLoaded = true;
        selectWaveform(&fCurrentSample->sourceInfo.buffer, fCurrentSample->sourceStart);
        pluginUpdate.notify(this, PluginUpdate::kSampleLoaded);
        pluginUpdate.notify(this, PluginUpdate::kParametersChanged);

        renderSample();
    }
}

void WAIVESampler::loadPreset(Preset preset)
{
    if (fCurrentSample == nullptr)
        return;

    renderSampleLock = true;
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
    fCurrentSample->preset.assign(preset.presetName);
    renderSampleLock = false;

    pluginUpdate.notify(this, PluginUpdate::kSampleLoaded);

    renderSample();
    pluginUpdate.notify(this, PluginUpdate::kParametersChanged);
}

void WAIVESampler::selectWaveform(std::vector<float> *source, int start)
{
    // if (!fSourceLoaded)
    //     return;

    if (start >= source->size())
        start = 0;

    // fSampleLoaded = false;
    if (fCurrentSample == nullptr)
        newSample();

    fCurrentSample->sourceStart = start;
    // fSampleLoaded = true;

    renderSample();
    triggerPreview();
    pluginUpdate.notify(this, PluginUpdate::kSampleUpdated);
}

void WAIVESampler::renderSample()
{
    // LOG_LOCATION
    if (fCurrentSample == nullptr || !fCurrentSample->sourceInfo.sourceLoaded || renderSampleLock)
        return;

    std::lock_guard<std::mutex> lock(samplePlayerMtx);

    Source *sourceInfo = &fCurrentSample->sourceInfo;
    fCurrentSample->sampleLength = ampEnvGen.getLength(fCurrentSample->sustainLength);
    fCurrentSample->sampleLength = std::min(fCurrentSample->sampleLength, sourceInfo->length - fCurrentSample->sourceStart);

    auto minmax = std::minmax_element(
        &sourceInfo->buffer[fCurrentSample->sourceStart],
        &sourceInfo->buffer[fCurrentSample->sourceStart + fCurrentSample->sampleLength]);
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
    int deltaEnvLength = std::floor(sampleRate / 20.f);
    if (d_isZero(fCurrentSample->percussiveBoost))
        deltaEnvLength = -1;

    float y = 0.0f;
    size_t index = 0;
    float indexF = 0.0f;

    for (size_t i = 0; i < fCurrentSample->sampleLength; i++)
    {
        index = std::round(indexF);
        if (fCurrentSample->sourceStart + index >= sourceInfo->length)
        {
            fCurrentSample->sampleLength = i;
            editorPreviewPlayer->length = i;
            break;
        }

        if (i < deltaEnvLength)
            delta = interpolate((float)i / (float)deltaEnvLength, deltaStart, fCurrentSample->pitch, 1.0f, true);
        else
            delta = fCurrentSample->pitch;

        y = sourceInfo->buffer[fCurrentSample->sourceStart + index];
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

    // fSampleLoaded = true;
    pluginUpdate.notify(this, PluginUpdate::kSampleUpdated);
}

void WAIVESampler::loadSamplePlayer(int spIndex, std::shared_ptr<SampleInfo> info)
{
    SamplePlayer *sp = &samplePlayers[spIndex];

    if (info == nullptr)
    {
        sp->active = false;
        return;
    }

    sp->state = PlayState::STOPPED;
    sp->ptr = 0;
    sp->active = false;
    sp->sampleInfo = info;

    WaveformLoaderTask *task = new WaveformLoaderTask(fmt::format("loadSamplePlayer:{:d}", spIndex), tempBuffer, &tempBufferMutex, sd.getFullSamplePath(info), sampleRate);
    taskManager.start(task);
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
    if (slot >= samplePlayers.size())
    {
        std::cerr << "WAIVESampler::loadSlot slot " << slot << " is out of range " << samplePlayers.size() << std::endl;
        return;
    }
    std::shared_ptr<SampleInfo> info = sd.findSample(id);
    loadSamplePlayer(slot, info);
}

void WAIVESampler::generateCurrentSampleName(const std::string base)
{
    if (fCurrentSample == nullptr)
        return;

    std::stringstream test(fCurrentSample->sourceInfo.tagString);
    std::string segment;
    std::vector<std::string> seglist;

    while (std::getline(test, segment, '|'))
        seglist.push_back(segment);

    std::string tags = "";
    if (seglist.size() > 0)
        tags += "_" + seglist[0];

    if (seglist.size() > 1)
        tags += "_" + seglist[1];

    sd.renameSample(fCurrentSample, sd.getNewSampleName(base + tags + ".wav"));
}

void WAIVESampler::triggerPreview()
{
    // if (!fSampleLoaded)
    //     return;

    if (editorPreviewPlayer->state == PlayState::STOPPED)
    {
        editorPreviewPlayer->state = PlayState::TRIGGERED;
        editorPreviewPlayer->active = true;
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

    if (taskName == "loadSourceBuffer")
    {
        if (pTask->isCancelled())
        {
            fCurrentSample->sourceInfo.fp = "";
            fCurrentSample->sourceInfo.length = 0;
            pNf->release();
            return;
        }

        std::lock_guard<std::mutex> lock(tempBufferMutex);

        size_t sourceLength = tempBuffer->size();
        fCurrentSample->sourceInfo.buffer.resize(sourceLength);
        fCurrentSample->sourceInfo.length = sourceLength;

        std::copy(tempBuffer->begin(), tempBuffer->begin() + sourceLength, fCurrentSample->sourceInfo.buffer.begin());

        if (!fCurrentSample)
            newSample();

        if (sourceLength > 0)
        {
            fCurrentSample->sourceInfo.sourceLoaded = true;
            fCurrentSample->tagString = fCurrentSample->sourceInfo.tagString;
            selectWaveform(&fCurrentSample->sourceInfo.buffer, fCurrentSample->sourceStart);

            pluginUpdate.notify(this, PluginUpdate::kSourceLoaded);
            taskManager.start(new FeatureExtractorTask(&fCurrentSample->sourceInfo, getSampleRate()));
            renderSample();
        }
        else
        {
            std::cerr << "Source failed to load" << std::endl;
            fCurrentSample->sourceInfo.fp = "";
            fCurrentSample->tagString = "";
            fCurrentSample->sourceInfo.length = 0;
        }
    }
    else if (taskName == "loadSourcePreview")
    {
        stopSourcePreview();
        std::lock_guard<std::mutex> spLock(samplePlayerMtx);
        std::lock_guard<std::mutex> tbLock(tempBufferMutex);

        size_t size = tempBuffer->size();
        if (size == 0)
        {
            std::cout << "loadSourcePreview: Not able to load\n";
            pNf->release();
            return;
        }
        sourcePreviewWaveform->resize(size);
        std::copy(tempBuffer->begin(), tempBuffer->begin() + size, sourcePreviewWaveform->begin());

        sourcePreviewPlayer->waveform = sourcePreviewWaveform;
        sourcePreviewPlayer->startAt = 0;
        sourcePreviewPlayer->length = size;
        sourcePreviewPlayer->active = true;
        sourcePreviewPlayer->state = PlayState::TRIGGERED;
    }
    else if (taskName.find("loadSamplePlayer") != std::string::npos)
    {
        // int
        int spIndex = 0; // = std::stoi(taskName.);
        size_t colonPos = taskName.find(':');
        if (colonPos != std::string::npos)
            spIndex = std::stoi(taskName.substr(colonPos + 1));
        else
            throw std::runtime_error("loadSamplePlayer task name in wrong format: " + taskName);

        SamplePlayer *sp = &samplePlayers[spIndex];

        std::lock_guard<std::mutex> spLock(samplePlayerMtx);
        std::lock_guard<std::mutex> tbLock(tempBufferMutex);

        size_t size = tempBuffer->size();

        if (size == 0)
        {
            sp->active = false;
            std::cerr << "WAIVESampler::loadSamplePlayer: Sample waveform length 0" << std::endl;
        }
        else
        {
            sp->length = size;
            sp->waveform->resize(size);
            std::copy(tempBuffer->begin(), tempBuffer->begin() + size, sp->waveform->begin());
            sp->active = true;
        }
        // sp.sampleInfo->tagString.assign(info->tagString); // no idea why we must do this only for tagString...

        pluginUpdate.notify(this, PluginUpdate::kSlotLoaded);
    }

    pNf->release();
}

void WAIVESampler::onTaskFailed(Poco::TaskFailedNotification *pNf)
{
    Poco::Task *pTask = pNf->task();
    if (pTask == nullptr)
    {
        std::cerr << "Error: Task is null" << std::endl;
        return;
    }
    std::cout << "WAIVESampler::onTaskFailed: " << pTask->name() << std::endl;
    std::cout << "  Reason: " << pNf->reason().message() << std::endl;
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

const char *WAIVESampler::pluginUpdateToString(PluginUpdate update) const
{
    switch (update)
    {
    case kSourceLoading:
        return "kSourceLoading";
    case kSourceLoaded:
        return "kSourceLoaded";
    case kSourceUpdated:
        return "kSourceUpdated";
    case kSampleLoading:
        return "kSampleLoading";
    case kSampleLoaded:
        return "kSampleLoaded";
    case kSampleUpdated:
        return "kSampleUpdated";
    case kSampleCleared:
        return "kSampleCleared";
    case kSampleAdded:
        return "kSampleAdded";
    case kSlotLoaded:
        return "kSlotLoaded";
    case kParametersChanged:
        return "kParametersChanged";
    default:
        break;
    }

    return "UNKNOWN";
}

Plugin *createPlugin()
{
    return new WAIVESampler();
}

bool saveWaveform(const char *fp, const float *buffer, sf_count_t size, int sampleRate)
{
    // TODO: make into task
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