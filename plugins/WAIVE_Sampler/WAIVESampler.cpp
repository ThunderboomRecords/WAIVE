#include "WAIVESampler.hpp"

START_NAMESPACE_DISTRHO

WAIVESampler::WAIVESampler() : Plugin(kParameterCount, 0, 0),
                               sampleRate(getSampleRate()),
                               fSampleLoaded(false),
                               fNormalisationRatio(1.0f),
                               fSourceLoaded(false),
                               fCurrentSample(nullptr),
                               ampEnvGen(getSampleRate(), ENV_TYPE::ADSR, {10, 50, 0.7, 100}),
                               gist({512, (int)getSampleRate()}),
                               server(SimpleUDPServer("127.0.0.1", 8000))
{
    if (isDummyInstance())
        std::cout << "** dummy instance" << std::endl;

    srand(time(NULL));

    sd = SampleDatabase();

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

    uint8_t defaultMidiMap[] = {36, 38, 47, 50, 43, 42, 46, 51, 49};
    for (int i = 0; i < 8; i++)
        samplePlayers[i].midi = defaultMidiMap[i];

    newSample();

    // OSC Test
    int len = tosc_writeMessage(oscBuffer, sizeof(oscBuffer), "/WAIVE_Sampler", "s", "testing");
    tosc_printOscBuffer(oscBuffer, len);
    server.sendMessage(oscBuffer, len);
}

WAIVESampler::~WAIVESampler()
{
    std::cout << "closing WAIVESampler..." << std::endl;
    sd.saveSamples();
}

void WAIVESampler::initParameter(uint32_t index, Parameter &parameter)
{
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
        parameter.ranges.def = 1.0f;
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
        parameter.ranges.max = 500.0f;
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
    default:
        std::cerr << "getParameter: Unknown parameter index: " << index << "  " << std::endl;
        break;
    }

    return val;
}

void WAIVESampler::setParameterValue(uint32_t index, float value)
{
    // std::cout << "WAIVESampler::setParameterValue : " << index << " -> " << value << std::endl;
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
    samplePlayerMtx.lock();

    tosc_bundle bundle;
    tosc_writeBundle(&bundle, 1, oscBuffer, sizeof(oscBuffer));
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
            // std::cout << "noteOn: " << type << " " << note << ":" << velocity << std::endl;
            // printf("type: %02X data1: %02X (%d) data2: %02X (%d) \n", type, note, note, velocity, velocity);

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

                        // send OSC
                        bundle_length += tosc_writeNextMessage(
                            &bundle,
                            "/WAIVE_Sampler",
                            "si",
                            samplePlayers[j].sampleInfo->name.c_str(),
                            note);
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

    if (bundle_length)
        server.sendMessage(oscBuffer, tosc_getBundleLength(&bundle));

    samplePlayerMtx.unlock();
}

void WAIVESampler::loadSource(const char *fp)
{
    LOG_LOCATION
    fSourceLoaded = false;
    addToUpdateQueue(kSourceLoading);

    fSourceLength = loadWaveform(fp, &fSourceWaveform);

    if (fSourceLength > 0)
    {
        fSourceLoaded = true;
        fCurrentSample->source = std::string(fp);

        getOnsets();

        addToUpdateQueue(kSourceLoaded);
    }
    else
    {
        std::cerr << "Source failed to load\n";
    }
}

int WAIVESampler::loadWaveform(const char *fp, std::vector<float> *buffer)
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
    //           << " fileSampleRate: " << fileSampleRate
    //           << " (sampleRate: " << sampleRate << ")\n";

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
            std::cerr << "Failed to convert sample rate: " << src_strerror(result) << std::endl;
        else
            std::cout << "sample rate conversion complete. ratio: " << src_data.src_ratio << std::endl;
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

void WAIVESampler::newSample()
{
    LOG_LOCATION

    // TODO: save current sample before creating a new one?

    time_t current_time = time(NULL);
    std::string name = fmt::format("Sample{:d}.wav", current_time % 10000);

    std::shared_ptr<SampleInfo> s(new SampleInfo(current_time, name, SAMPLE_DIR, true));
    if (fCurrentSample != nullptr)
    {
        s->pitch = fCurrentSample->pitch;
        s->percussiveBoost = fCurrentSample->percussiveBoost;
        s->volume = fCurrentSample->volume;
        s->filterCutoff = fCurrentSample->filterCutoff;
        s->filterResonance = fCurrentSample->filterResonance;
        s->filterType = fCurrentSample->filterType;
        s->sustainLength = fCurrentSample->sustainLength;

        s->source = fCurrentSample->source;
        s->sourceStart = fCurrentSample->sourceStart;
        s->embedX = fCurrentSample->embedX;
        s->embedY = fCurrentSample->embedY;
    }
    s->adsr = ADSR_Params(ampEnvGen.getADSR());
    s->saved = false;

    fCurrentSample = s;

    getEmbedding();

    std::cout << fCurrentSample->getId() << std::endl;

    addToUpdateQueue(kParametersChanged);
}

void WAIVESampler::addCurrentSampleToLibrary()
{
    sd.addToLibrary(fCurrentSample);
    saveWaveform(sd.getSamplePath(fCurrentSample).c_str(), &(editorPreviewWaveform->at(0)), fCurrentSample->sampleLength);
}

void WAIVESampler::loadPreview(int id)
{
    loadSlot(9, id);

    if (mapPreviewPlayer->state == PlayState::STOPPED && mapPreviewPlayer->active)
        mapPreviewPlayer->state = PlayState::TRIGGERED;
}

void WAIVESampler::loadSample(int id)
{
    loadSample(sd.findSample(id));
}

void WAIVESampler::loadSample(std::shared_ptr<SampleInfo> s)
{
    // LOG_LOCATION
    if (s == nullptr)
        return;

    // save/update current sample
    // addToLibrary();

    fCurrentSample = s;
    std::cout << s->name << std::endl;
    fSampleLoaded = false;

    // load source (if avaliable)
    if (fCurrentSample->waive)
    {
        loadSource(fCurrentSample->source.c_str());
        ampEnvGen.setADSR(fCurrentSample->adsr);
        setParameterValue(kSampleVolume, fCurrentSample->volume);
        setParameterValue(kSamplePitch, fCurrentSample->pitch);
        setParameterValue(kAmpAttack, fCurrentSample->adsr.attack);
        setParameterValue(kAmpDecay, fCurrentSample->adsr.decay);
        setParameterValue(kAmpSustain, fCurrentSample->adsr.sustain);
        setParameterValue(kAmpRelease, fCurrentSample->adsr.release);
        setParameterValue(kSustainLength, fCurrentSample->sustainLength);
        selectWaveform(&fSourceWaveform, fCurrentSample->sourceStart);
    }
    else
    {
        // TODO: load sample directly (hide sample controls?)
        fCurrentSample->sampleLength = loadWaveform(sd.getSamplePath(fCurrentSample).c_str(), editorPreviewWaveform);
    }

    fSampleLoaded = true;
    renderSample();

    addToUpdateQueue(kSampleLoaded);
    addToUpdateQueue(kParametersChanged);
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
    addToUpdateQueue(kSampleUpdated);
}

void WAIVESampler::renderSample()
{
    // LOG_LOCATION
    if (!fSampleLoaded || fCurrentSample == nullptr)
        return;

    samplePlayerMtx.lock();

    fSampleLoaded = false;
    fCurrentSample->sampleLength = ampEnvGen.getLength(fCurrentSample->sustainLength);
    fCurrentSample->sampleLength = std::min(fCurrentSample->sampleLength, fSourceLength - fCurrentSample->sourceStart);
    editorPreviewPlayer->length = fCurrentSample->sampleLength;
    editorPreviewPlayer->ptr = 0;
    editorPreviewPlayer->active = true;

    auto minmax = std::minmax_element(
        &fSourceWaveform[fCurrentSample->sourceStart],
        &fSourceWaveform[fCurrentSample->sourceStart + fCurrentSample->sampleLength]);
    float normaliseRatio = std::max(-(*minmax.first), *minmax.second);
    if (std::abs(normaliseRatio) <= 0.0001f)
        normaliseRatio = 1.0f;

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
            break;

        amp = ampEnvGen.getValue();
    }

    getEmbedding();
    fSampleLoaded = true;
    addToUpdateQueue(kSampleUpdated);
    samplePlayerMtx.unlock();
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
    int length = loadWaveform(sd.getSamplePath(info).c_str(), &buffer);

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

    addToUpdateQueue(kSlotLoaded);
    samplePlayerMtx.unlock();
}

void WAIVESampler::loadSlot(int slot, int id)
{
    std::shared_ptr<SampleInfo> info = sd.findSample(id);
    loadSamplePlayer(info, samplePlayers.at(slot), samplePlayerWaveforms.at(slot));
}

void WAIVESampler::triggerPreview()
{
    if (samplePlayers[8].state == PlayState::STOPPED)
    {
        samplePlayers[8].state = PlayState::TRIGGERED;
        samplePlayers[8].active = true;
    }
}

void WAIVESampler::getEmbedding()
{
    // TODO: placeholder is random for now
    int x = ((long)fCurrentSample->getId() * 987) % 10000;
    int y = ((long)fCurrentSample->getId() * 12345) % 10000;

    float embedX = (float)x / 5000.0f - 1.0f;
    float embedY = (float)y / 5000.0f - 1.0f;

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

void WAIVESampler::getOnsets()
{
    if (fSourceLength == 0)
        return;

    int frame = 0;
    fSourceFeatures.clear();

    while (frame < fSourceLength - gist.getAudioFrameSize())
    {
        gist.processAudioFrame(std::vector<float>(fSourceWaveform.begin() + frame, fSourceWaveform.begin() + frame + gist.getAudioFrameSize()));
        float onset = gist.spectralDifferenceHWR();
        if (onset > 100.0f)
        {
            fSourceFeatures.push_back({FeatureType::Onset,
                                       "onset",
                                       onset,
                                       frame,
                                       frame});

            printf(" - Onset detected at %d (onset: %.2f)\n", frame, onset);
        }

        frame += gist.getAudioFrameSize();
    }
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
    ampEnvGen.sampleRate = newSampleRate;
}

Plugin *createPlugin()
{
    return new WAIVESampler();
}

END_NAMESPACE_DISTRHO