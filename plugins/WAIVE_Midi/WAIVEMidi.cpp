#include "WAIVEMidi.hpp"


START_NAMESPACE_DISTRHO

using namespace torch::indexing;

WAIVEMidi::WAIVEMidi() : Plugin(kParameterCount, 0, 0),
                         fThreshold(0.7),
                         fSixteenth(0),
                         ticks_per_beat(1920)
{

    s_map[0] = 0;
    for(int i=1; i<9; i++)
    {
        s_map[i] = s_map[i-1] + max_events[i-1];
    }

    std::stringstream model_stream;
    std::cout << "loading models...";
    try 
    {
        model_stream.write((char *) score_decoder, score_decoder_len);
        score_decoder_model = torch::jit::load(model_stream);
        score_decoder_model.eval();
        
        model_stream.str("");
        model_stream.write((char *) score_encoder, score_encoder_len);
        score_encoder_model = torch::jit::load(model_stream);
        score_encoder_model.eval();

        model_stream.str("");
        model_stream.write((char *) groove_decoder, groove_decoder_len);
        groove_decoder_model = torch::jit::load(model_stream);
        groove_decoder_model.eval();

        model_stream.str("");
        model_stream.write((char *) full_groove_model, full_groove_model_len);
        full_model = torch::jit::load(model_stream);
        full_model.eval();

        std::cout << " done\n";
    }
    catch (const c10::Error& e) 
    {
        std::cerr << " error loading the model\n";
        return;
    }


    // load distributions
    score_m = torch::from_blob(score_means, {64});
    score_s = torch::from_blob(score_stds, {64});
    groove_m = torch::from_blob(groove_means, {32});
    groove_s = torch::from_blob(groove_stds, {32});

    // generate 
    generateScore();
    generateGroove();
    generateFullPattern();
    
}

void WAIVEMidi::initParameter(uint32_t index, Parameter &parameter)
{
    switch(index)
    {
        case kThreshold:
            parameter.name = "Threshold";
            parameter.symbol = "threshold";
            parameter.ranges.min = 0.0f;
            parameter.ranges.max = 1.0f;
            parameter.ranges.def = 0.7f;
            parameter.hints = kParameterIsAutomatable;
            break;
        default:
            break;
    }
}

float WAIVEMidi::getParameterValue(uint32_t index) const
{
    float val = 0.0f;
    switch(index)
    {
        case kThreshold:
            val = fThreshold;
            break;
        default:
            break;
    }

    return val;
}

void WAIVEMidi::setParameterValue(uint32_t index, float value)
{
    switch(index)
    {
        case kThreshold:
            fThreshold = value;
            break;
        default:
            break;
    }
}

void WAIVEMidi::setState(const char *key, const char *value){
    printf("WAIVEMidi::setState\n");
    printf("  %s: %s\n", key, value);
    if(std::strcmp(key, "score") == 0){
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
    switch(index)
    {
        default:
        break;
    }
}

void WAIVEMidi::run(
    const float **,              // incoming audio
    float **,                    // outgoing audio
    uint32_t numFrames,          // size of block to process
    const MidiEvent *midiEvents, // MIDI pointer
    uint32_t midiEventCount      // Number of MIDI events in block
)
{
    const TimePosition& timePos(getTimePosition());

    for(uint32_t i=0; i<midiEventCount; ++i) 
    {
        MidiEvent me = midiEvents[i];
        writeMidiEvent(me);
    }

    static bool wasPlaying = false;
    static int loopTick = 0;

    wasPlaying = timePos.playing;

    if(!timePos.playing) return;

    if(!timePos.bbt.valid)
    {
        fSixteenth = 0;
        return;
    }

    float tick = timePos.bbt.tick;
    float beat = timePos.bbt.beat - 1.0f;
    float tpb = timePos.bbt.ticksPerBeat;

    if(ticks_per_beat != tpb) {
        ticks_per_beat = tpb;
        computeNotes();
    }

    for(uint32_t i=0; i < numFrames; i++)
    {
        loopTick++;
        if(loopTick >= tpb*4) loopTick = 0;

        // find any midi events at t = loopTick
        // and send

        // frames =/= ticks! need to convert... 
    }
}

void WAIVEMidi::encodeScore()
{
    std::vector<torch::jit::IValue> input;
    torch::Tensor score_t = torch::zeros({16, 9});

    for(int i=0; i<16; i++){
        for(int j=0; j<9; j++){
            score_t.index_put_({i, j}, fScore[i][j]);
        }
    }
    input.push_back(score_t.reshape({144}));

    score_z = score_encoder_model.forward(input).toTensor();

    generateFullPattern();
}

void WAIVEMidi::generateScore()
{
    std::vector<torch::jit::IValue> input;
    torch::Tensor s_z = torch::randn({64});
    s_z = score_s * s_z + score_m;
    score_z = s_z;

    input.push_back(s_z);
    torch::Tensor score = score_decoder_model.forward(input).toTensor().reshape({16, 9});

    auto score_a = score.accessor<float, 2>();
    for(int i=0; i<16; i++){
        for(int j=0; j<9; j++){
            fScore[i][j] = score_a[i][j];
        }
    }
}

void WAIVEMidi::generateGroove()
{
    std::vector<torch::jit::IValue> input;
    torch::Tensor g_z = torch::randn({32});
    g_z = groove_s * g_z + groove_m;
    groove_z = g_z;

    input.push_back(g_z);
    torch::Tensor groove = groove_decoder_model.forward(input).toTensor().reshape({48, 3});

    auto groove_a = groove.accessor<float, 2>();
    for(int i=0; i<48; i++){
        for(int j=0; j<3; j++){
            fGroove[i][j] = groove_a[i][j];
        }
    }
}

void WAIVEMidi::generateFullPattern()
{
    std::vector<torch::jit::IValue> input;
    torch::Tensor z = torch::cat({score_z, groove_z}, 0);
    auto z_a = z.accessor<float, 1>();
    std::cout << "full_z = [";
    for(int i=0; i<96; i++){
        std::cout << z_a[i] << ", ";
    }
    std::cout << "]\n";

    input.push_back(z);
    torch::Tensor pattern = full_model.forward(input).toTensor().reshape({16, 30, 3});

    auto pattern_a = pattern.accessor<float, 3>();
    for(int i=0; i<16; i++){
        for(int j=0; j<30; j++){
            for(int k=0; k<3; k++){
                fDrumPattern[i][j][k] = pattern_a[i][j][k];
            }
        }
    }
    computeNotes();
    // std::cout << "full_decoder output: \n" << pattern.index({Slice(0, 8)}) << std::endl;
}

/**
   For sorting Note structs by time, noteOn/Off, then by 
   midiNote number
*/
bool compareNotes(Note n0, Note n1)
{
    if(n0.tick < n1.tick)
    {
        return true;
    }
    else if(n0.tick > n1.tick)
    {
        return false;
    }
    
    if(!n0.noteOn && n1.noteOn)
    {
        return true;
    }
    else if(n0.noteOn && !n1.noteOn)
    {
        return false;
    }

    return n0.midiNote < n1.midiNote;
}

void WAIVEMidi::computeNotes()
{
    int tp16th = ticks_per_beat / 4;

    std::cout << "WAIVEMidi::computeNotes()\n  tp16th: " << tp16th << std::endl;

    notes.clear();

    // Dim 0: Instrument
    // Dim 1: notes
    std::vector< std::vector<Note> > newNotes;

    newNotes.resize(9);


    for(int j=0; j<9; j++){
        // first collect all the noteOn events:
        for(int i=0; i<16; i++){
            for(int k=0; k<max_events[j]; k++){
                int index = s_map[j] + k;

                if(fDrumPattern[i][index][0] < 0.3f) break;

                float vel = fDrumPattern[i][index][1];
                vel = 0.5f * (vel + 1.0f);

                uint8_t velocity = (uint8_t) (vel * 255.0f);

                float offset = fDrumPattern[i][index][2];

                int tickOn = (int) ((i + offset) * tp16th);
                tickOn = std::max(tickOn, 0);

                Note noteOn = {
                    tickOn, velocity, midiMap[j], 9, true
                };

                newNotes[j].push_back(noteOn);
            }
        }

        if(newNotes[j].size() == 0) continue;

        // make sure noteOns are in temporal order
        std::sort(newNotes[j].begin(), newNotes[j].end(), compareNotes);

        // add all the noteOff events
        int nNotes = newNotes[j].size();
        for(int i=0; i<nNotes - 1; i++)
        {
            int thisOnTick = newNotes[j][i].tick;
            int nextOnTick = newNotes[j][i+1].tick;
            int noteOffTick = std::min(thisOnTick+tp16th, nextOnTick);

            Note noteOff = {
                noteOffTick, 0, newNotes[j][i].midiNote, 9, false
            };
            newNotes[j].push_back(noteOff);
        }

        // add final noteOff
        int noteOffTick = std::min(newNotes[j][nNotes-1].tick + tp16th, tp16th*16*4 - 1);
        Note noteOff = {
            noteOffTick, 0, newNotes[j][nNotes-1].midiNote, 9, false
        };
        newNotes[j].push_back(noteOff);

        // then reorder again
        std::sort(newNotes[j].begin(), newNotes[j].end(), compareNotes);

        std::cout << "Instrument " << j << std::endl;
        for(int i=0; i<newNotes[j].size(); i++)
        {
            Note n = newNotes[j][i];
            if(n.noteOn)
                printf("  % 4d: %02d noteOn  velocity %d \n", n.tick, n.midiNote, n.velocity);
            else
                printf("  % 4d: %02d noteOff velocity %d \n", n.tick, n.midiNote, n.velocity);


            notes.push_back(n);
        }
        std::cout << std::endl;
    }

    std::sort(notes.begin(), notes.end(), compareNotes);

}

void WAIVEMidi::sampleRateChanged(double newSampleRate)
{
    sampleRate = newSampleRate;
}


Plugin *createPlugin()
{
    return new WAIVEMidi();
}

END_NAMESPACE_DISTRHO