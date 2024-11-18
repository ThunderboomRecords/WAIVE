#include "WAIVEMidi.hpp"

START_NAMESPACE_DISTRHO

WAIVEMidi::WAIVEMidi() : Plugin(kParameterCount, 0, 0),
                         fThreshold(0.7f),
                         ticks_per_beat(1920),
                         loopTick(0.0),
                         progress(0.0f),
                         score_genre(0),
                         groove_genre(0),
                         hold_update(false),
                         quantize(false)
{

    sampleRate = getSampleRate();

    s_map[0] = 0;
    for (int i = 1; i < 9; i++)
        s_map[i] = s_map[i - 1] + max_events[i - 1];

    seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::cout << seed << std::endl;

    generator = std::default_random_engine(seed);
    distribution = std::normal_distribution<float>(0.0f, 1.0f);

    std::cout << "loading models...";
    try
    {
        auto info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

        sessionOptions.SetIntraOpNumThreads(1);
        sessionOptions.SetInterOpNumThreads(1);

        mScoreEncoder = std::make_unique<Ort::Session>(mEnv, (void *)score_encoder_onnx_start, score_encoder_onnx_size, sessionOptions);
        mScoreDecoder = std::make_unique<Ort::Session>(mEnv, (void *)score_decoder_onnx_start, score_decoder_onnx_size, sessionOptions);

        mGrooveEncoder = std::make_unique<Ort::Session>(mEnv, (void *)groove_encoder_onnx_start, groove_encoder_onnx_size, sessionOptions);
        mGrooveDecoder = std::make_unique<Ort::Session>(mEnv, (void *)groove_decoder_onnx_start, groove_decoder_onnx_size, sessionOptions);

        mFullDecoder = std::make_unique<Ort::Session>(mEnv, (void *)full_groove_model_onnx_start, full_groove_model_onnx_size, sessionOptions);

        assert(mScoreEncoder);
        assert(mScoreDecoder);

        assert(mGrooveEncoder);
        assert(mGrooveDecoder);

        assert(mFullDecoder);

        // SCORE Model
        mScoreEncoderInputShape = GetInputShapes(mScoreEncoder);
        mScoreDecoderInputShape = GetInputShapes(mScoreDecoder);
        mScoreEncoderOutputShape = GetOutputShapes(mScoreEncoder);
        mScoreDecoderOutputShape = GetOutputShapes(mScoreDecoder);

        mScoreEncoderInputNames = GetInputNames(mScoreEncoder);
        mScoreDecoderInputNames = GetInputNames(mScoreDecoder);
        mScoreEncoderOutputNames = GetOutputNames(mScoreEncoder);
        mScoreDecoderOutputNames = GetOutputNames(mScoreDecoder);

        mScoreZ.resize(mScoreDecoderInputShape[0][0]);
        mScoreInput.resize(mScoreEncoderInputShape[0][0]);
        mScoreOutput.resize(mScoreDecoderOutputShape[0][0]);

        std::fill(mScoreZ.begin(), mScoreZ.end(), 0.0f);
        std::fill(mScoreInput.begin(), mScoreInput.end(), 0.0f);
        std::fill(mScoreOutput.begin(), mScoreOutput.end(), 0.0f);

        mScoreZTensor.push_back(Ort::Value::CreateTensor<float>(info, mScoreZ.data(), mScoreZ.size(), mScoreDecoderInputShape[0].data(), mScoreDecoderInputShape[0].size()));
        mScoreInputTensor.push_back(Ort::Value::CreateTensor<float>(info, mScoreInput.data(), mScoreInput.size(), mScoreEncoderInputShape[0].data(), mScoreEncoderInputShape[0].size()));
        mScoreOutputTensor.push_back(Ort::Value::CreateTensor<float>(info, mScoreOutput.data(), mScoreOutput.size(), mScoreDecoderOutputShape[0].data(), mScoreDecoderOutputShape[0].size()));

        // GROOVE Model
        mGrooveEncoderInputShape = GetInputShapes(mGrooveEncoder);
        mGrooveDecoderInputShape = GetInputShapes(mGrooveDecoder);
        mGrooveEncoderOutputShape = GetOutputShapes(mGrooveEncoder);
        mGrooveDecoderOutputShape = GetOutputShapes(mGrooveDecoder);

        mGrooveEncoderInputNames = GetInputNames(mGrooveEncoder);
        mGrooveDecoderInputNames = GetInputNames(mGrooveDecoder);
        mGrooveEncoderOutputNames = GetOutputNames(mGrooveEncoder);
        mGrooveDecoderOutputNames = GetOutputNames(mGrooveDecoder);

        mGrooveZ.resize(mGrooveDecoderInputShape[0][0]);
        mGrooveInput.resize(mGrooveEncoderInputShape[0][0]);
        mGrooveOutput.resize(mGrooveDecoderOutputShape[0][0]);

        std::fill(mGrooveZ.begin(), mGrooveZ.end(), 0.0f);
        std::fill(mGrooveInput.begin(), mGrooveInput.end(), 0.0f);
        std::fill(mGrooveOutput.begin(), mGrooveOutput.end(), 0.0f);

        mGrooveZTensor.push_back(Ort::Value::CreateTensor<float>(info, mGrooveZ.data(), mGrooveZ.size(), mGrooveDecoderInputShape[0].data(), mGrooveDecoderInputShape[0].size()));
        mGrooveInputTensor.push_back(Ort::Value::CreateTensor<float>(info, mGrooveInput.data(), mGrooveInput.size(), mGrooveEncoderInputShape[0].data(), mGrooveEncoderInputShape[0].size()));
        mGrooveOutputTensor.push_back(Ort::Value::CreateTensor<float>(info, mGrooveOutput.data(), mGrooveOutput.size(), mGrooveDecoderOutputShape[0].data(), mGrooveDecoderOutputShape[0].size()));

        // FULL DECODER Model
        mFullDecoderInputShape = GetInputShapes(mFullDecoder);
        mFullDecoderOutputShape = GetOutputShapes(mFullDecoder);

        mFullDecoderInputNames = GetInputNames(mFullDecoder);
        mFullDecoderOutputNames = GetOutputNames(mFullDecoder);

        mFullZ.resize(mFullDecoderInputShape[0][0]);
        mFullOutput.resize(mFullDecoderOutputShape[0][0]);

        std::fill(mFullZ.begin(), mFullZ.end(), 0.0f);
        std::fill(mFullOutput.begin(), mFullOutput.end(), 0.0f);

        mFullZTensor.push_back(Ort::Value::CreateTensor<float>(info, mFullZ.data(), mFullZ.size(), mFullDecoderInputShape[0].data(), mFullDecoderInputShape[0].size()));
        mFullOutputTensor.push_back(Ort::Value::CreateTensor<float>(info, mFullOutput.data(), mFullOutput.size(), mFullDecoderOutputShape[0].data(), mFullDecoderOutputShape[0].size()));

        std::cout << " done\n";

        // PrintModelDetails("mScoreEncoder", mScoreEncoderInputNames, mScoreEncoderOutputNames, mScoreEncoderInputShape, mScoreEncoderOutputShape);
        // PrintModelDetails("mScoreDecoder", mScoreDecoderInputNames, mScoreDecoderOutputNames, mScoreDecoderInputShape, mScoreDecoderOutputShape);

        // PrintModelDetails("mGrooveEncoder", mGrooveEncoderInputNames, mGrooveEncoderOutputNames, mGrooveEncoderInputShape, mGrooveEncoderOutputShape);
        // PrintModelDetails("mGrooveDecoder", mGrooveDecoderInputNames, mGrooveDecoderOutputNames, mGrooveDecoderInputShape, mGrooveDecoderOutputShape);

        // PrintModelDetails("mFullDecoder", mFullDecoderInputNames, mFullDecoderOutputNames, mFullDecoderInputShape, mFullDecoderOutputShape);

        std::cout << std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << " error loading models\n";
        return;
    }

    midiNotes.resize(9);
    for (int i = 0; i < 9; i++)
    {
        midiNotes[i] = midiMap[i];
        fThresholds[i] = fThreshold;
    }

    notes.reserve(16 * 9 * 3);
    notesPointer = notes.begin();

    generateScore();
    generateGroove();
    generateFullPattern();
}

void WAIVEMidi::initParameter(uint32_t index, Parameter &parameter)
{
    // std::cout << "WAIVEMidi::initParameter index " << index << std::endl;
    int instrument = 0;
    char nameFmt[] = "Complexity %d";
    switch (index)
    {
    case kThreshold:
        parameter.name = "Complexity";
        parameter.symbol = "complexity";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.5f;
        parameter.hints = kParameterIsAutomatable;
        break;
    case kScoreX:
        parameter.name = "ScoreX";
        parameter.symbol = "scoreX";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.5f;
        parameter.hints = kParameterIsAutomatable;
        break;
    case kScoreY:
        parameter.name = "ScoreY";
        parameter.symbol = "scoreY";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.5f;
        parameter.hints = kParameterIsAutomatable;
        break;
    case kGrooveX:
        parameter.name = "GrooveX";
        parameter.symbol = "grooveX";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.5f;
        parameter.hints = kParameterIsAutomatable;
        break;
    case kGrooveY:
        parameter.name = "GrooveY";
        parameter.symbol = "grooveY";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.5f;
        parameter.hints = kParameterIsAutomatable;
        break;
    case kGrooveNew:
        parameter.name = "New Groove";
        parameter.symbol = "new_groove";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.0f;
        parameter.hints = kParameterIsTrigger | kParameterIsAutomatable;
        break;
    case kGrooveVar:
        parameter.name = "Variation Groove";
        parameter.symbol = "variation_groove";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.0f;
        parameter.hints = kParameterIsTrigger | kParameterIsAutomatable;
        break;
    case kScoreNew:
        parameter.name = "New Score";
        parameter.symbol = "new_score";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.0f;
        parameter.hints = kParameterIsTrigger | kParameterIsAutomatable;
        break;
    case kScoreVar:
        parameter.name = "Variation Score";
        parameter.symbol = "variation_score";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.0f;
        parameter.hints = kParameterIsTrigger | kParameterIsAutomatable;
        break;
    case kThreshold1:
    case kThreshold2:
    case kThreshold3:
    case kThreshold4:
    case kThreshold5:
    case kThreshold6:
    case kThreshold7:
    case kThreshold8:
    case kThreshold9:
        instrument = index - kThreshold1;
        parameter.name = fmt::format("Complexity {:d}", instrument + 1).c_str();
        parameter.symbol = fmt::format("complexity{:d}", instrument + 1).c_str();
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.8f;
        parameter.hints = kParameterIsAutomatable;
        break;
    default:
        break;
    }
}

float WAIVEMidi::getParameterValue(uint32_t index) const
{
    float val = 0.0f;
    switch (index)
    {
    case kThreshold:
        val = fThreshold;
        break;
    case kScoreX:
        val = fScoreX;
        break;
    case kScoreY:
        val = fScoreY;
        break;
    case kGrooveX:
        val = fGrooveX;
        break;
    case kGrooveY:
        val = fGrooveY;
        break;
    case kScoreNew:
    case kScoreVar:
    case kGrooveNew:
    case kGrooveVar:
        val = 0.0f;
        break;
    case kThreshold1:
    case kThreshold2:
    case kThreshold3:
    case kThreshold4:
    case kThreshold5:
    case kThreshold6:
    case kThreshold7:
    case kThreshold8:
    case kThreshold9:
        val = fThresholds[index - kThreshold1];
        break;
    default:
        break;
    }

    return val;
}

void WAIVEMidi::setParameterValue(uint32_t index, float value)
{
    switch (index)
    {
    case kThreshold:
        fThreshold = value;

        hold_update = true;
        for (int i = 0; i < 9; i++)
            setParameterValue(kThreshold1 + i, value);
        // fThresholds[i] = fThreshold;
        hold_update = false;

        generateFullPattern();
        break;
    case kScoreX:
        fScoreX = value;
        break;
    case kScoreY:
        fScoreY = value;
        break;
    case kGrooveX:
        fGrooveX = value;
        break;
    case kGrooveY:
        fGrooveY = value;
        break;
    case kGrooveNew:
        if (value != 1.f)
            break;
        generateGroove();
        generateFullPattern();
        break;
    case kGrooveVar:
        if (value != 1.f)
            break;
        variationGroove();
        generateFullPattern();
        break;
    case kScoreNew:
        if (value != 1.f)
            break;
        generateScore();
        generateFullPattern();
        break;
    case kScoreVar:
        if (value != 1.f)
            break;
        variationScore();
        generateFullPattern();
        break;
    case kThreshold1:
    case kThreshold2:
    case kThreshold3:
    case kThreshold4:
    case kThreshold5:
    case kThreshold6:
    case kThreshold7:
    case kThreshold8:
    case kThreshold9:
        fThresholds[index - kThreshold1] = value;
        if (!hold_update)
            generateFullPattern();
        break;
    default:
        break;
    }
}

void WAIVEMidi::setState(const char *key, const char *value)
{
    printf("WAIVEMidi::setState\n");
    printf("  %s: %s\n", key, value);
    if (std::strcmp(key, "score") == 0)
    {
        encodeScore();
    }
}

String WAIVEMidi::getState(const char *key) const
{
    String retString = String("undefined state");
    return retString;
}

void WAIVEMidi::initState(unsigned int index, String &stateKey, String &defaultStateValue)
{
    switch (index)
    {
    default:
        break;
    }
}

void WAIVEMidi::allNotesOff(uint32_t frame)
{
    std::set<uint8_t>::iterator it;

    MidiEvent me;
    me.size = 3;
    me.frame = frame;
    me.data[0] = 0x80;
    me.data[2] = 0;

    for (it = triggered.begin(); it != triggered.end(); it++)
    {
        me.data[1] = *it;
        writeMidiEvent(me);
    }

    triggered.clear();
}

void WAIVEMidi::run(
    const float **,              // incoming audio
    float **,                    // outgoing audio
    uint32_t numFrames,          // size of block to process
    const MidiEvent *midiEvents, // MIDI pointer
    uint32_t midiEventCount      // Number of MIDI events in block
)
{
    const TimePosition &timePos(getTimePosition());

    for (uint32_t i = 0; i < midiEventCount; ++i)
    {
        MidiEvent me = midiEvents[i];
        writeMidiEvent(me);
    }

    static bool wasPlaying = false;

    if (wasPlaying && !timePos.playing)
    {
        notesPointer = notes.begin();
        loopTick = 0.0;
        progress = 0.0f;
        allNotesOff(0);
    }

    wasPlaying = timePos.playing;

    if (!timePos.bbt.valid || !timePos.playing)
    {
        return;
    }

    float tick = timePos.bbt.tick;
    float beat = timePos.bbt.beat - 1.0f;
    float tpb = timePos.bbt.ticksPerBeat;
    double ticksPerLoop = tpb * 4.0;
    double samplesPerBeat = (60.0f * sampleRate) / timePos.bbt.beatsPerMinute;
    double samplesPerTick = samplesPerBeat / tpb;
    double ticksPerSample = tpb / samplesPerBeat;

    progress = loopTick / ticksPerLoop;

    if (ticks_per_beat != tpb)
    {
        ticks_per_beat = tpb;
        computeNotes();
    }

    MidiEvent me;
    me.size = 3;

    std::lock_guard<std::mutex> lk(noteMtx);

    for (uint32_t i = 0; i < numFrames; i++)
    {
        me.frame = i;
        while (notesPointer < notes.end() && (double)(*notesPointer).tick <= loopTick)
        {
            me.data[1] = (*notesPointer).midiNote;

            if ((*notesPointer).noteOn)
            {
                if (triggered.count((*notesPointer).midiNote))
                {
                    // note still on, send noteOff first
                    me.data[0] = 0x80;
                    me.data[2] = 0;
                    writeMidiEvent(me);
                }

                me.data[0] = 0x90;
                me.data[2] = (*notesPointer).velocity;
                triggered.insert((*notesPointer).midiNote);
            }
            else
            {
                me.data[0] = 0x80;
                me.data[2] = 0;
                triggered.erase((*notesPointer).midiNote);
            }

            writeMidiEvent(me);
            notesPointer++;
        }

        loopTick += ticksPerSample;
        if (loopTick >= ticksPerLoop)
        {
            loopTick = 0.0;
            notesPointer = notes.begin();
            allNotesOff(i);
        }
    }
}

void WAIVEMidi::encodeScore()
{
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            mScoreInput[j + i * 9] = fScore[i][j];
        }
    }

    const char *inputNamesCstrs[] = {mScoreEncoderInputNames[0].c_str()};
    const char *outputNamesCstrs[] = {mScoreEncoderOutputNames[0].c_str()};

    mScoreEncoder->Run(
        mRunOptions,
        inputNamesCstrs,
        mScoreInputTensor.data(),
        mScoreInputTensor.size(),
        outputNamesCstrs,
        mScoreZTensor.data(),
        mScoreZTensor.size());

    generateFullPattern();
}

void WAIVEMidi::generateScore()
{
    for (size_t i = 0; i < mScoreZ.size(); i++)
    {
        float z = distribution(generator);
        z = z * score_genre_stds[score_genre][i % 64] + score_genre_means[score_genre][i % 64];
        // z = z * score_stds[i % 64] + score_means[i % 64];
        mScoreZ[i] = z;
    }

    computeScore();
}

void WAIVEMidi::variationScore()
{
    for (size_t i = 0; i < mScoreZ.size(); i++)
    {
        float z = distribution(generator);
        z = z * score_stds[i % 64] * 0.1f;
        mScoreZ[i] += z;
    }

    computeScore();
}

void WAIVEMidi::computeScore()
{
    const char *inputNamesCstrs[] = {mScoreDecoderInputNames[0].c_str()};
    const char *outputNamesCstrs[] = {mScoreDecoderOutputNames[0].c_str()};

    mScoreDecoder->Run(
        mRunOptions,
        inputNamesCstrs,
        mScoreZTensor.data(),
        mScoreZTensor.size(),
        outputNamesCstrs,
        mScoreOutputTensor.data(),
        mScoreOutputTensor.size());

    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            fScore[i][j] = mScoreOutput[j + i * 9];
        }
    }
}

void WAIVEMidi::encodeGroove()
{
    std::fill(mGrooveInput.begin(), mGrooveInput.end(), 0.0f);
    std::vector<GrooveEvent>::iterator grooveEvents = fGroove.begin();
    for (; grooveEvents != fGroove.end(); grooveEvents++)
    {
        float velocity = 2.0f * ((*grooveEvents).velocity - 0.5f);
        float position = (*grooveEvents).position;
        int sixteenth = (int)std::clamp(std::round(position * 16.0f), 0.0f, 15.0f);
        float offset = position * 16.0f - (float)sixteenth;

        int index = sixteenth * 3 * 3;
        int j = 0;
        if (mGrooveInput[index] == 0.0f)
            j = 0;
        else if (mGrooveInput[index + 3] == 0.0f)
            j = 1;
        else
            j = 2;

        index += j * 3;

        mGrooveInput[index + 0] = 1.0f;
        mGrooveInput[index + 1] = velocity;
        mGrooveInput[index + 2] = offset;
    }

    const char *inputNamesCstrs[] = {mGrooveEncoderInputNames[0].c_str()};
    const char *outputNamesCstrs[] = {mGrooveEncoderOutputNames[0].c_str()};

    mGrooveEncoder->Run(
        mRunOptions,
        inputNamesCstrs,
        mGrooveInputTensor.data(),
        mGrooveInputTensor.size(),
        outputNamesCstrs,
        mGrooveZTensor.data(),
        mGrooveZTensor.size());

    generateFullPattern();
}

void WAIVEMidi::generateGroove()
{
    for (size_t i = 0; i < mGrooveZ.size(); i++)
    {
        float z = distribution(generator);
        z = z * groove_genre_stds[groove_genre][i % 32] + groove_genre_means[groove_genre][i % 32];
        // z = z * groove_stds[i % 32] + groove_means[i % 32];
        mGrooveZ[i] = z;
    }

    computeGroove();
}

void WAIVEMidi::variationGroove()
{
    for (size_t i = 0; i < mGrooveZ.size(); i++)
    {
        float z = distribution(generator);
        z = z * groove_stds[i % 32] * 0.1f;
        mGrooveZ[i] += z;
    }

    computeGroove();
}

void WAIVEMidi::computeGroove()
{
    const char *inputNamesCstrs[] = {mGrooveDecoderInputNames[0].c_str()};
    const char *outputNamesCstrs[] = {mGrooveDecoderOutputNames[0].c_str()};

    mGrooveDecoder->Run(
        mRunOptions,
        inputNamesCstrs,
        mGrooveZTensor.data(),
        mGrooveZTensor.size(),
        outputNamesCstrs,
        mGrooveOutputTensor.data(),
        mGrooveOutputTensor.size());

    fGroove.clear();
    // [i, j, k]  ->  [(i*3 + j) * 3 + k]
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int k = i * 3 + j;
            int index = k * 3;

            if (mGrooveOutput[index] < 0.3f)
                break;

            float velocity = 0.5f * mGrooveOutput[index + 1] + 0.5f;
            float offset = mGrooveOutput[index + 2];
            float position = ((float)i + offset) / 16.0f;
            position = std::clamp(position, 0.0f, 1.0f);

            GrooveEvent g = {position, velocity};
            fGroove.push_back(g);
        }
    }

    std::sort(fGroove.begin(), fGroove.end(), compareGrooveEvents);
}

void WAIVEMidi::generateFullPattern()
{
    mFullZ.clear();
    for (const float z : mScoreZ)
        mFullZ.push_back(z);

    for (const float z : mGrooveZ)
        mFullZ.push_back(z);

    const char *inputNamesCstrs[] = {mFullDecoderInputNames[0].c_str()};
    const char *outputNamesCstrs[] = {mFullDecoderOutputNames[0].c_str()};

    mFullDecoder->Run(
        mRunOptions,
        inputNamesCstrs,
        mFullZTensor.data(),
        mFullZTensor.size(),
        outputNamesCstrs,
        mFullOutputTensor.data(),
        mFullOutputTensor.size());

    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 30; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                int index = k + 3 * (j + 30 * i);
                fDrumPattern[i][j][k] = mFullOutput[index];
            }
        }
    }

    computeNotes();
}

void WAIVEMidi::setMidiNote(int instrument, uint8_t midi)
{
    midiNotes[instrument] = midi;
    computeNotes();
}

void WAIVEMidi::computeNotes()
{
    std::lock_guard<std::mutex> lk(noteMtx);

    int tp16th = ticks_per_beat / 4;

    int nextTick = -1;
    if (notesPointer != notes.end())
        nextTick = (*notesPointer).tick;

    // std::cout << "WAIVEMidi::computeNotes()\n  tp16th: " << tp16th << std::endl;

    notes.clear();

    // Dim 0: Instrument
    // Dim 1: notes
    std::vector<std::vector<Note>> newNotes;
    newNotes.resize(9);

    for (int j = 0; j < 9; j++)
    {
        // first collect all the noteOn events:
        for (int i = 0; i < 16; i++)
        {
            for (int k = 0; k < max_events[j]; k++)
            {
                int index = s_map[j] + k;

                if (fDrumPattern[i][index][0] < (1.f - fThresholds[j]))
                    break;

                float vel = fDrumPattern[i][index][1];
                vel = 0.5f * (vel + 1.0f);

                uint8_t velocity = (uint8_t)(vel * 255.0f);

                float offset = fDrumPattern[i][index][2];
                if (quantize)
                    offset = offset < .5f ? 0.f : 1.f;

                int tickOn = (int)(((float)i + offset) * tp16th);
                tickOn = std::max(tickOn, 0);

                Note noteOn = {
                    tickOn, velocity, midiNotes[j], 9, true, j};

                newNotes[j].push_back(noteOn);
            }
        }

        if (newNotes[j].size() == 0)
            continue;

        // make sure noteOns are in temporal order
        std::sort(newNotes[j].begin(), newNotes[j].end(), compareNotes);

        // add all the noteOff events
        int nNotes = newNotes[j].size();
        for (int i = 0; i < nNotes - 1; i++)
        {
            int thisOnTick = newNotes[j][i].tick;
            int nextOnTick = newNotes[j][i + 1].tick;
            int noteOffTick = std::min(thisOnTick + tp16th, nextOnTick);

            Note noteOff = {
                noteOffTick, 0, newNotes[j][i].midiNote, 9, false, j};
            newNotes[j].push_back(noteOff);
        }

        // add final noteOff
        int noteOffTick = std::min(newNotes[j][nNotes - 1].tick + tp16th, tp16th * 16 * 4 - 1);
        Note noteOff = {
            noteOffTick, 0, newNotes[j][nNotes - 1].midiNote, 9, false, j};
        newNotes[j].push_back(noteOff);

        // then reorder again
        std::sort(newNotes[j].begin(), newNotes[j].end(), compareNotes);

        int prevOnTick = -1;
        // std::cout << "Instrument " << j << std::endl;
        for (int i = 0; i < newNotes[j].size(); i++)
        {
            Note n = newNotes[j][i];

            // avoid instantaneous notes
            if (n.noteOn)
            {
                if (n.tick == prevOnTick)
                    continue;
                prevOnTick = n.tick;
            }

            // if(n.noteOn)
            //     printf("  % 4d: %02d noteOn  velocity %d \n", n.tick, n.midiNote, n.velocity);
            // else
            //     printf("  % 4d: %02d noteOff velocity %d \n", n.tick, n.midiNote, n.velocity);

            notes.push_back(n);
        }
        // std::cout << std::endl;
    }

    std::sort(notes.begin(), notes.end(), compareNotes);

    // advance pointer to reach time when computeNotes was called
    notesPointer = notes.begin();
    while (notesPointer != notes.end() && (*notesPointer).tick <= nextTick)
        notesPointer++;
}

void WAIVEMidi::sampleRateChanged(double newSampleRate)
{
    std::cout << "sampleRateChanged: " << newSampleRate << std::endl;
    sampleRate = newSampleRate;
}

Plugin *createPlugin()
{
    return new WAIVEMidi();
}

END_NAMESPACE_DISTRHO
