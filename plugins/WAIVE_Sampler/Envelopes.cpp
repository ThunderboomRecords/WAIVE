#include "Envelopes.hpp"

float interpolate(float p, float a, float b, float power = 1.0f, bool clamp = true)
{
    if (a == b)
        return a;

    float p2 = p;

    if (power != 1.0f)
        std::pow(p2, power);

    if (clamp)
        p2 = std::clamp(p2, 0.0f, 1.0f);

    return p2 * (b - a) + a;
}

EnvGen::EnvGen(int sampleRate, ENV_TYPE type, ADSR_Params adsr)
    : adsr(adsr),
      sampleRate(sampleRate),
      type(type)
{
    reset();
    calculateStages();
}

EnvGen::EnvGen(int sampleRate, ENV_TYPE type, float p1, float p2, float p3, float p4)
{
    switch (type)
    {
    case ADSR:
        EnvGen(sampleRate, type, {p1, p2, p3, p4});
        break;
    case AD:
        EnvGen(sampleRate, type, {p1, p2, 0.0f, 0.0f});
        break;
    case SR:
        EnvGen(sampleRate, type, {0.0f, 0.0f, p1, p2});
        break;
    case D:
        EnvGen(sampleRate, type, {0.0f, 0.0f, 1.0f, p1});
        break;
    default:
        break;
    }
}

void EnvGen::setADSR(ADSR_Params adsr)
{
    EnvGen::adsr = adsr;
    calculateStages();
}

ADSR_Stage EnvGen::getStage()
{
    return stage;
}

void EnvGen::calculateStages()
{
    float t = 0;

    if (type == ENV_TYPE::ADSR || type == ENV_TYPE::AD)
    {
        startAttack = (int)(t * sampleRate / 1000.f);
        t += adsr.attack;
    }

    t += 1;

    if (type == ENV_TYPE::ADSR || type == ENV_TYPE::AD || type == ENV_TYPE::D)
    {
        startDecay = (int)(t * sampleRate / 1000.f);
        t += adsr.decay;
    }

    t += 1;

    if (type == ENV_TYPE::AD || type == ENV_TYPE::D)
    {
        endStep = (int)(t * sampleRate / 1000.f);
        adsr.sustain = 0.0f;
        startSustain = endStep;
    }

    if (type == ENV_TYPE::ADSR || type == ENV_TYPE::SR)
    {
        startSustain = (int)(t * sampleRate / 1000.f);
        endStep = INT_MAX; // endStep calculated on Sustain release.
    }
}

void EnvGen::reset()
{
    active = false;
    step = 0;
    value = 0.0f;

    switch (type)
    {
    case ADSR:
    case AD:
        stage = ADSR_Stage::ATTACK;
        break;
    case SR:
        stage = ADSR_Stage::SUSTAIN;
        value = adsr.sustain;
        break;
    case D:
        stage = ADSR_Stage::DECAY;
        value = 1.0f;
    default:
        break;
    }
}

float EnvGen::getValue()
{
    return value;
}

void EnvGen::trigger()
{
    active = true;
}

void EnvGen::release()
{
    if (type == ENV_TYPE::ADSR || type == ENV_TYPE::SR)
    {
        stage = ADSR_Stage::RELEASE;
        startRelease = step;
        endStep = step + (int)(adsr.release * sampleRate / 1000.f);
        value = adsr.sustain; // immediately jump to sustain value to start release
    }
}

void EnvGen::process()
{
    if (step >= endStep)
    {
        active = false;
        return;
    }

    // check if stage must change
    switch (type)
    {
    case ADSR:
        if (step >= startSustain && stage == ADSR_Stage::ATTACK)
            stage = ADSR_Stage::SUSTAIN;
        else if (step >= startDecay && stage == ADSR_Stage::ATTACK)
            stage = ADSR_Stage::DECAY;
        break;
    case AD:
        if (step >= startDecay && stage == ADSR_Stage::ATTACK)
            stage = ADSR_Stage::DECAY;
        break;
    case SR:
    case D:
    default:
        break;
    }

    // compute next value
    float p;
    switch (stage)
    {
    case ADSR_Stage::ATTACK:
        p = (float)(step - startAttack) / (startDecay - startAttack);
        value = interpolate(p, 0.0f, 1.0f);
        break;
    case ADSR_Stage::DECAY:
        p = (float)(step - startDecay) / (startSustain - startDecay);
        value = interpolate(p, 1.0f, adsr.sustain);
        break;
    case ADSR_Stage::RELEASE:
        p = (float)(step - startRelease) / (endStep - startRelease);
        value = interpolate(p, adsr.sustain, 0.0f);
        break;
    default:
        break;
    }

    step++;
}

int EnvGen::getLength(float sustain_time = 0.0f)
{
    float ms = 0.0f;
    switch (type)
    {
    case ENV_TYPE::ADSR:
        ms = adsr.attack + adsr.decay + adsr.release + sustain_time;
        break;
    case ENV_TYPE::AD:
        ms = adsr.attack + adsr.decay;
        break;
    case ENV_TYPE::SR:
        ms = sustain_time + adsr.release;
        break;
    case ENV_TYPE::D:
        ms = adsr.decay;
        break;
    default:
        break;
    }

    return (int)(sampleRate * ms / 1000.0f);
}
