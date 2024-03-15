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

}

void WAIVEMidi::sampleRateChanged(double newSampleRate) {}

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

    // std::cout << "full_decoder output: \n" << pattern.index({Slice(0, 8)}) << std::endl;
}

Plugin *createPlugin()
{
    return new WAIVEMidi();
}

END_NAMESPACE_DISTRHO