#ifndef ENVELOPES_HPP_INCLUDED
#define ENVELOPES_HPP_INCLUDED

#include <iostream>
#include <math.h>
#include <climits>
#include <algorithm>

/*
    attack: time in ms
    decay: time in ms
    sustain: gain between 0.0 and 1.0
    release: time in ms
*/
struct ADSR_Params
{
    float attack = 100.f;
    float decay = 100.f;
    float sustain = 1.0f;
    float release = 100.f;
};

enum ADSR_Stage
{
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
};

enum ENV_TYPE
{
    ADSR,
    AD,
    SR,
    D
};

float interpolate(float p, float a, float b, float power, bool clamp);

class EnvGen
{
public:
    EnvGen(int sampleRate, ENV_TYPE type, ADSR_Params *adsr);

    void reset();
    void process();
    void trigger();
    void release();

    float getValue();
    int getLength(float sustainLength);
    ADSR_Stage getStage();

    void setADSR(ADSR_Params *adsr);
    void calculateStages();

    int sampleRate;
    bool active;
    float power;

private:
    ADSR_Stage stage;
    ADSR_Params *adsr;
    ENV_TYPE type;
    float value;
    int step;

    int startAttack, startDecay, startRelease, startSustain, endStep;
};

#endif