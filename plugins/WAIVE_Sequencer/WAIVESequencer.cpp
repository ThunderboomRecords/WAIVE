#include "WAIVESequencer.hpp"

START_NAMESPACE_DISTRHO

WAIVESequencer::WAIVESequencer() : Plugin(kParameterCount, 0, kStateCount),
                                   fThreshold(0.4f),
                                   ticks_per_beat(1920),
                                   loopTick(0.0),
                                   progress(0.0f),
                                   score_genre(NUM_GROOVE_GENRES - 1),
                                   groove_genre(NUM_GROOVE_GENRES - 1),
                                   hold_update(false),
                                   quantize(false)
{

    sampleRate = getSampleRate();
    ticks_per_16th = ticks_per_beat / 4;

    s_map[0] = 0;
    for (int i = 1; i < 9; i++)
        s_map[i] = s_map[i - 1] + max_events[i - 1];

    seed = std::chrono::system_clock::now().time_since_epoch().count();

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

    triggerGenerated.reserve(16 * 9 * 3);
    triggerUser.reserve(16 * 9 * 3);
    notesOut.reserve(16 * 9 * 3 * 3);
    notesPointer = notesOut.begin();

    generateScore();
    generateGroove();
    generateFullPattern();
}

void WAIVESequencer::initParameter(uint32_t index, Parameter &parameter)
{
    int instrument = 0;
    char nameFmt[] = "Complexity %d";
    switch (index)
    {
    case kThreshold:
        parameter.name = "Complexity";
        parameter.symbol = "complexity";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.4f;
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
    case kScoreGenre:
        parameter.name = "Score Genre";
        parameter.symbol = "score_genre";
        parameter.ranges.min = 0;
        parameter.ranges.max = NUM_SCORE_GENRES;
        parameter.ranges.def = NUM_SCORE_GENRES - 1;
        parameter.hints = kParameterIsInteger | kParameterIsAutomatable;
        break;
    case kGrooveGenre:
        parameter.name = "Groove Genre";
        parameter.symbol = "groove_genre";
        parameter.ranges.min = 0;
        parameter.ranges.max = NUM_GROOVE_GENRES;
        parameter.ranges.def = NUM_GROOVE_GENRES - 1;
        parameter.hints = kParameterIsInteger | kParameterIsAutomatable;
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
        parameter.ranges.def = 0.4f;
        parameter.hints = kParameterIsAutomatable;
        break;
    case kMidi1:
    case kMidi2:
    case kMidi3:
    case kMidi4:
    case kMidi5:
    case kMidi6:
    case kMidi7:
    case kMidi8:
    case kMidi9:
        instrument = index - kMidi1;
        parameter.name = fmt::format("Midi Note {:d}", instrument + 1).c_str();
        parameter.symbol = fmt::format("midi{:d}", instrument + 1).c_str();
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 127.f;
        parameter.ranges.def = static_cast<float>(midiMap[instrument]);
        parameter.hints = kParameterIsAutomatable | kParameterIsInteger;
        break;
    default:
        break;
    }
}

float WAIVESequencer::getParameterValue(uint32_t index) const
{
    float val = 0.0f;
    switch (index)
    {
    case kThreshold:
        val = fThreshold;
        break;
    case kScoreNew:
    case kScoreVar:
    case kGrooveNew:
    case kGrooveVar:
        val = 0.0f;
        break;
    case kScoreGenre:
        val = score_genre;
        break;
    case kGrooveGenre:
        val = groove_genre;
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
    case kMidi1:
    case kMidi2:
    case kMidi3:
    case kMidi4:
    case kMidi5:
    case kMidi6:
    case kMidi7:
    case kMidi8:
    case kMidi9:
        val = midiNotes[index - kMidi1];
        break;
    default:
        break;
    }

    return val;
}

void WAIVESequencer::setParameterValue(uint32_t index, float value)
{
    std::cout << "WAIVESequencer::setParameterValue " << parameterIndexToString(index) << ": " << value << std::endl;
    switch (index)
    {
    case kThreshold:
        fThreshold = value;
        hold_update = true;
        for (int i = 0; i < 9; i++)
            setParameterValue(kThreshold1 + i, value);
        hold_update = false;
        generateTriggers();
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
    case kScoreGenre:
        score_genre = (int)value;
        // Disabling regenerate Score and Groove to allow saved state to persist
        // TODO: fix this (state is restored before parameters, so setting the genres
        // overrides loaded state...)

        // generateScore();
        // generateFullPattern();
        break;
    case kGrooveGenre:
        groove_genre = (int)value;
        // generateGroove();
        // generateFullPattern();
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
            generateTriggers();
        break;
    case kMidi1:
    case kMidi2:
    case kMidi3:
    case kMidi4:
    case kMidi5:
    case kMidi6:
    case kMidi7:
    case kMidi8:
    case kMidi9:
        setMidiNote(index - kMidi1, static_cast<uint8_t>(value) + 1);
        break;
    default:
        break;
    }
}

void WAIVESequencer::initState(uint32_t index, State &state)
{
    // std::cout << "WAIVESequencer::initState " << index << std::endl;
    switch (index)
    {
    case kStateScoreZ:
        state.key = "score-z";
        state.defaultValue = "";
        break;
    case kStateGrooveZ:
        state.key = "groove-z";
        state.defaultValue = "";
        break;
    case kStateDrumPattern:
        state.key = "drum-pattern";
        state.defaultValue = "";
        break;
    case kStateUserTriggers:
        state.key = "user-triggers";
        state.defaultValue = "";
        break;
    case kStateGeneratedTriggers:
        state.key = "generated-triggers";
        state.defaultValue = "";
        break;
    default:
        break;
    }
}

String WAIVESequencer::getState(const char *key) const
{
    std::cout << "WAIVESequencer::getState " << key << std::endl;

    String retString("unrecognised state");

    if (std::strcmp(key, "score-z") == 0)
    {
        std::ostringstream oss;
        for (size_t i = 0; i < mScoreZ.size(); i++)
        {
            if (i != 0)
                oss << ",";
            oss << mScoreZ[i];
        }

        retString = String(oss.str().c_str());
    }
    else if (std::strcmp(key, "groove-z") == 0)
    {
        std::ostringstream oss;
        for (size_t i = 0; i < mGrooveZ.size(); i++)
        {
            if (i != 0)
                oss << ",";
            oss << mGrooveZ[i];
        }

        retString = String(oss.str().c_str());
    }
    else if (std::strcmp(key, "drum-pattern") == 0)
    {
        std::ostringstream oss;
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 30; j++)
            {
                for (int k = 0; k < 3; k++)
                {
                    oss << i << ";" << j << ";" << k << ";" << fDrumPattern[i][j][k] << "\n";
                }
            }
        }

        retString = String(oss.str().c_str());
    }
    else if (std::strcmp(key, "user-triggers") == 0)
    {
        std::ostringstream oss;
        for (const auto &trigger : triggerUser)
            oss << trigger->serialize() << "\n";

        retString = String(oss.str().c_str());
    }
    else if (std::strcmp(key, "generated-triggers") == 0)
    {
        std::ostringstream oss;

        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 30; j++)
            {
                if (fDrumPatternTriggers[i][j] == nullptr)
                    continue;
                oss << i << ";" << j << ";" << fDrumPatternTriggers[i][j]->serialize() << "\n";
            }
        }

        retString = String(oss.str().c_str());
    }

    // std::cout << retString << std::endl;

    return retString;
}

void WAIVESequencer::setState(const char *key, const char *value)
{
    std::cout << "WAIVESequencer::setState " << key << std::endl;

    if (std::strcmp(key, "score") == 0)
    {
        // encodeScore();
    }
    else if (std::strcmp(key, "export") == 0)
    {
        exportMidiFile(notesOut, value);
    }
    else if (std::strcmp(key, "score-z") == 0)
    {
        if (std::strlen(value) == 0)
            return;

        mScoreZ.clear();
        std::istringstream iss(value);
        std::string line;
        while (std::getline(iss, line, ','))
            mScoreZ.push_back(std::stof(line));

        // std::cout << "Length of mScoreZ: " << mScoreZ.size() << std::endl;
        computeScore();
    }
    else if (std::strcmp(key, "groove-z") == 0)
    {
        if (std::strlen(value) == 0)
            return;

        mGrooveZ.clear();
        std::istringstream iss(value);
        std::string line;
        while (std::getline(iss, line, ','))
            mGrooveZ.push_back(std::stof(line));

        // std::cout << "Length of mGrooveZ: " << mGrooveZ.size() << std::endl;
        computeGroove();
    }
    else if (std::strcmp(key, "drum-pattern") == 0)
    {
        if (std::strlen(value) == 0)
            return;

        std::istringstream iss(value);
        std::string line;
        while (std::getline(iss, line))
        {
            std::istringstream issLine(line);
            std::string iS, jS, kS, tS;
            std::getline(issLine, iS, ';');
            std::getline(issLine, jS, ';');
            std::getline(issLine, kS, ';');
            std::getline(issLine, tS, ';');

            size_t i = std::stoi(iS);
            size_t j = std::stoi(jS);
            size_t k = std::stoi(kS);

            fDrumPattern[i][j][k] = std::stof(tS);
        }
    }
    else if (std::strcmp(key, "user-triggers") == 0)
    {
        if (std::strlen(value) == 0)
            return;

        triggerUser.clear();
        std::istringstream iss(value);
        std::string line;
        while (std::getline(iss, line))
            triggerUser.push_back(std::make_shared<Trigger>(Trigger::deserialize(line)));

        computeNotes();
    }
    else if (std::strcmp(key, "generated-triggers") == 0)
    {
        if (std::strlen(value) == 0)
            return;

        triggerGenerated.clear();

        std::istringstream iss(value);
        std::string line;
        while (std::getline(iss, line))
        {
            std::istringstream issLine(line);
            std::string iS, jS, tS;
            std::getline(issLine, iS, ';');
            std::getline(issLine, jS, ';');
            std::getline(issLine, tS, ';');

            size_t i = std::stoi(iS);
            size_t j = std::stoi(jS);

            fDrumPatternTriggers[i][j] = std::make_shared<Trigger>(Trigger::deserialize(tS));
        }
        generateTriggers();
    }
}

void WAIVESequencer::allNotesOff(uint32_t frame)
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

void WAIVESequencer::run(
    const float **inputs,        // incoming audio
    float **outputs,             // outgoing audio
    uint32_t numFrames,          // size of block to process
    const MidiEvent *midiEvents, // MIDI pointer
    uint32_t midiEventCount      // Number of MIDI events in block
)
{
    const TimePosition &timePos(getTimePosition());

    // TODO: investigate why the plugin needs audio in/out?
    // for now pass audio through
    for (size_t i = 0; i < numFrames; ++i)
        for (size_t j = 0; j < 2; ++j)
            outputs[j][i] = inputs[j][i];

    for (uint32_t i = 0; i < midiEventCount; ++i)
    {
        MidiEvent me = midiEvents[i];
        writeMidiEvent(me);
    }

    static bool wasPlaying = false;

    if (wasPlaying && !timePos.playing)
    {
        notesPointer = notesOut.begin();
        loopTick = 0.0;
        progress = 0.0f;
        allNotesOff(0);
    }

    wasPlaying = timePos.playing;

    if (!timePos.bbt.valid || !timePos.playing)
        return;

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
        ticks_per_16th = tpb / 4;
        computeNotes();
    }

    MidiEvent me;
    me.size = 3;

    std::lock_guard<std::mutex> lk(noteMtx);

    for (uint32_t i = 0; i < numFrames; i++)
    {
        me.frame = i;
        while (notesPointer < notesOut.end() && (double)(*notesPointer)->tick <= loopTick)
        {
            if (!(*notesPointer)->active)
            {
                notesPointer++;
                continue;
            }

            me.data[1] = (*notesPointer)->midiNote;

            if ((*notesPointer)->noteOn)
            {
                if (triggered.count((*notesPointer)->midiNote))
                {
                    // note still on, send noteOff first
                    me.data[0] = 0x80;
                    me.data[2] = 0;
                    writeMidiEvent(me);
                }

                me.data[0] = 0x90;
                me.data[2] = (*notesPointer)->velocity;
                triggered.insert((*notesPointer)->midiNote);
            }
            else
            {
                // check if note is on
                me.data[0] = 0x80;
                me.data[2] = 0;
                triggered.erase((*notesPointer)->midiNote);
            }

            writeMidiEvent(me);
            notesPointer++;
        }

        loopTick += ticksPerSample;
        if (loopTick >= ticksPerLoop)
        {
            loopTick = 0.0;
            notesPointer = notesOut.begin();
            allNotesOff(i);
        }
    }
}

void WAIVESequencer::encodeScore()
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

void WAIVESequencer::generateScore()
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

void WAIVESequencer::variationScore()
{
    for (size_t i = 0; i < mScoreZ.size(); i++)
    {
        float z = distribution(generator);
        z = z * score_stds[i % 64] * 0.1f;
        mScoreZ[i] += z;
    }

    computeScore();
}

void WAIVESequencer::computeScore()
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

void WAIVESequencer::encodeGroove()
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

void WAIVESequencer::generateGroove()
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

void WAIVESequencer::variationGroove()
{
    for (size_t i = 0; i < mGrooveZ.size(); i++)
    {
        float z = distribution(generator);
        z = z * groove_stds[i % 32] * 0.1f;
        mGrooveZ[i] += z;
    }

    computeGroove();
}

void WAIVESequencer::computeGroove()
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

void WAIVESequencer::generateFullPattern()
{
    std::cout << "WAIVESequencer::generateFullPattern()" << std::endl;

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

    for (int j = 0; j < 9; j++)
    {
        for (int i = 0; i < 16; i++)
        {
            for (int k = 0; k < max_events[j]; k++)
            {
                int index = s_map[j] + k;

                float vel = fDrumPattern[i][index][1];
                vel = 0.5f * (vel + 1.0f);

                uint8_t velocity = (uint8_t)(vel * 127.0f);

                float offset = fDrumPattern[i][index][2];
                float tickOnF = std::floor(std::max(0.f, ((float)i + offset) * ticks_per_16th));

                uint32_t tickOn = static_cast<uint32_t>(tickOnF);

                Trigger t = {
                    tickOn,
                    velocity,
                    j,
                };

                fDrumPatternTriggers[i][index] = std::make_shared<Trigger>(t);
            }
        }
    }

    generateTriggers();
}

void WAIVESequencer::generateTriggers()
{
    std::cout << "WAIVESequencer::generateTriggers()" << std::endl;
    triggerGenerated.clear();
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            for (int k = 0; k < max_events[j]; k++)
            {
                int index = s_map[j] + k;

                if (fDrumPattern[i][index][0] < (1.f - fThresholds[j]))
                    break;

                triggerGenerated.push_back(fDrumPatternTriggers[i][index]);
            }
        }
    }

    computeNotes();
}

void WAIVESequencer::setMidiNote(int instrument, uint8_t midi)
{
    midiNotes[instrument] = midi;
    computeNotes();
}

void WAIVESequencer::addNote(int instrument, int sixteenth, uint8_t velocity)
{
    // std::cout << "WAIVESequencer::addNote instrument " << instrument << " sixteenth " << sixteenth << " velocity " << (int)velocity << std::endl;
    if (instrument < 0 || instrument > 9 || sixteenth < 0)
        return;

    Trigger t = {sixteenth * ticks_per_16th, velocity, instrument};
    triggerUser.push_back(std::make_shared<Trigger>(t));

    computeNotes();
}

void WAIVESequencer::updateNote(std::shared_ptr<Note> note)
{
    if (note->user && !note->active)
        deleteNote(note);
    else
        computeNotes();
}

void WAIVESequencer::deleteNote(std::shared_ptr<Note> note)
{
    std::cout << "WAIVESequencer::deleteNote" << std::endl;

    std::shared_ptr<Trigger> trigger = note->trigger;
    if (trigger == nullptr)
    {
        std::cout << "Note is null" << std::endl;
        return;
    }

    std::vector<std::shared_ptr<Trigger>>::iterator position = std::find(triggerUser.begin(), triggerUser.end(), trigger);
    if (position != triggerUser.end())
    {
        std::cout << "Deleted note->trigger, rebuilding notes" << std::endl;
        triggerUser.erase(position);
        computeNotes();
    }
    else
        std::cout << "note->trigger not found in triggerUser" << std::endl;
}

void WAIVESequencer::createNoteOn(const std::vector<std::shared_ptr<Trigger>> &triggers, std::vector<std::shared_ptr<Note>> &notesNew, bool user)
{
    for (auto &trigger : triggers)
    {
        if (trigger->instrument < 0)
            continue;

        uint32_t onTick = trigger->tick;
        if (quantize)
            onTick = static_cast<uint32_t>(std::round(static_cast<float>(onTick) / static_cast<float>(ticks_per_16th)) * ticks_per_16th);

        // Create noteOn
        Note noteOn = {
            onTick,
            trigger->velocity,
            midiNotes[trigger->instrument],
            DRUM_CHANNEL,
            true,
            trigger->instrument,
            user,
            0,
            nullptr};

        noteOn.trigger = trigger;
        noteOn.active = trigger->active;

        notesNew.push_back(std::make_shared<Note>(noteOn));
    }
}

void WAIVESequencer::computeNotes()
{
    std::cout << "WAIVESequencer::computeNotes()" << std::endl;

    std::lock_guard<std::mutex> lk(noteMtx);

    // get tick of notesPointer
    uint32_t nextNoteTick = 0;
    if (notesPointer != notesOut.end())
        nextNoteTick = (*notesPointer)->tick;

    notesOut.clear();
    std::vector<std::shared_ptr<Note>> notesNew;

    createNoteOn(triggerGenerated, notesNew);
    createNoteOn(triggerUser, notesNew, true);

    std::sort(notesNew.begin(), notesNew.end(), compareNotes);
    std::reverse(notesNew.begin(), notesNew.end());

    // Clean up overlapping notes
    int lastNoteOn[9];
    std::fill_n(&lastNoteOn[0], 9, -1);

    for (auto noteOn : notesNew)
    {
        // Note noteOn = *it;
        int instrument = noteOn->instrument;
        if (instrument < 0)
            continue;

        uint32_t noteOffTick;

        if (lastNoteOn[instrument] == -1)
            noteOffTick = noteOn->tick + ticks_per_16th;
        else
            noteOffTick = std::min(static_cast<uint32_t>(lastNoteOn[instrument]), noteOn->tick + ticks_per_16th);

        noteOffTick = std::min(noteOffTick, ticks_per_beat * 4);

        Note noteOff = {
            noteOffTick,
            0,
            noteOn->midiNote,
            noteOn->channel,
            false,
            noteOn->instrument,
            noteOn->user,
            noteOn->offset,
            nullptr};

        noteOn->other = std::make_shared<Note>(noteOff);
        noteOff.active = noteOn->active;

        notesOut.push_back(std::make_shared<Note>(noteOff));

        lastNoteOn[instrument] = noteOn->tick;
        notesOut.push_back(noteOn);
    }

    std::reverse(notesOut.begin(), notesOut.end());
    std::sort(notesOut.begin(), notesOut.end(), compareNotes);

    // reset notePointer to last position
    notesPointer = notesOut.begin();
    uint32_t t = 0;
    while (t < nextNoteTick && notesPointer < notesOut.end())
    {
        notesPointer++;
        t = (*notesPointer)->tick;
    }
}

void WAIVESequencer::sampleRateChanged(double newSampleRate)
{
    std::cout << "sampleRateChanged: " << newSampleRate << std::endl;
    sampleRate = newSampleRate;
}

Plugin *createPlugin()
{
    return new WAIVESequencer();
}

END_NAMESPACE_DISTRHO
