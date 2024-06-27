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
    EnvGen(int sampleRate, ENV_TYPE type, ADSR_Params adsr);

    void reset();
    void process();
    void trigger();
    void release();

    float getValue() const;
    int getLength(float sustainLength) const;
    ADSR_Stage getStage() const;

    void setADSR(ADSR_Params adsr);
    ADSR_Params getADSR() const;

    void setAttack(float attack);
    float getAttack() const;
    void setDecay(float decay);
    float getDecay() const;
    void setSustain(float sustain);
    float getSustain() const;
    void setRelease(float release);
    float getRelease() const;

    void calculateStages();

    int sampleRate;
    bool active;
    float power;

private:
    ADSR_Params adsr;
    ADSR_Stage stage;
    ENV_TYPE type;
    float value;
    int step;

    int startAttack, startDecay, startRelease, startSustain, endStep;
};

#endif