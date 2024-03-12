#include "WAIVEMidi.hpp"


START_NAMESPACE_DISTRHO

using namespace torch::indexing;

WAIVEMidi::WAIVEMidi() : Plugin(kParameterCount, 0, 0),
                         fThreshold(0.7)
{

    std::stringstream model_stream;
    std::cout << "loading models...";
    try 
    {
        model_stream.write((char *) score_decoder, score_decoder_len);
        score_decoder_model = torch::jit::load(model_stream);
        score_decoder_model.eval();

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

void WAIVEMidi::setState(const char *key, const char *value){}

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

}

void WAIVEMidi::sampleRateChanged(double newSampleRate) {}

void WAIVEMidi::generateScore()
{
    std::vector<torch::jit::IValue> input;
    torch::Tensor s_z = torch::randn({64});
    s_z = score_s * s_z + score_m;
    score_z = s_z;

    input.push_back(s_z);
    torch::Tensor score = score_decoder_model.forward(input).toTensor().reshape({16, 9});
    std::cout << "score_decoder output: \n" << score.index({Slice(0, 8)}) << std::endl;

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
    std::cout << "groove_decoder output: \n" << groove.index({Slice(0, 8)}) << std::endl;

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

    input.push_back(z);
    pattern = full_model.forward(input).toTensor().reshape({16, 30, 3});

    std::cout << "full_decoder output: \n" << pattern.index({Slice(0, 8)}) << std::endl;
}

Plugin *createPlugin()
{
    return new WAIVEMidi();
}

END_NAMESPACE_DISTRHO