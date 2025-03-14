#include "Envelopes.hpp"

float interpolate(float p, float a, float b, float power = 1.0f, bool clamp = true)
{
    if (a == b)
        return a;

    float p2 = p;

    if (power != 1.0f)
        p2 = std::pow(p2, power);

    if (clamp)
        p2 = std::clamp(p2, 0.0f, 1.0f);

    return p2 * (b - a) + a;
}

EnvGen::EnvGen(int sampleRate, ENV_TYPE type, ADSR_Params adsr)
    : adsr(adsr),
      sampleRate(sampleRate),
      type(type),
      power(1.0f),
      startRelease(0)
{
    reset();
    calculateStages();
}

void EnvGen::setADSR(ADSR_Params adsr)
{
    EnvGen::adsr = adsr;
    calculateStages();
}

ADSR_Params EnvGen::getADSR() const
{
    return adsr;
}

void EnvGen::setAttack(float attack)
{
    adsr.attack = attack;
    calculateStages();
}

float EnvGen::getAttack() const
{
    return adsr.attack;
}

void EnvGen::setDecay(float decay)
{
    adsr.decay = decay;
    calculateStages();
}

float EnvGen::getDecay() const
{
    return adsr.decay;
}

void EnvGen::setSustain(float sustain)
{
    adsr.sustain = sustain;
    calculateStages();
}

float EnvGen::getSustain() const
{
    return adsr.sustain;
}

void EnvGen::setRelease(float release)
{
    adsr.release = release;
    calculateStages();
}

float EnvGen::getRelease() const
{
    return adsr.release;
}

ADSR_Stage EnvGen::getStage() const
{
    return stage;
}

void EnvGen::calculateStages()
{
    float t = 0;

    if (type == ENV_TYPE::ADSR || type == ENV_TYPE::AD)
    {
        startAttack = static_cast<int>(t * sampleRate / 1000.f);
        t += adsr.attack;
    }

    if (type == ENV_TYPE::ADSR || type == ENV_TYPE::AD || type == ENV_TYPE::D)
    {
        startDecay = static_cast<int>(t * sampleRate / 1000.f);
        t += adsr.decay;
    }

    if (type == ENV_TYPE::AD || type == ENV_TYPE::D)
    {
        endStep = static_cast<int>(t * sampleRate / 1000.f);
        startSustain = endStep;
    }

    if (type == ENV_TYPE::ADSR || type == ENV_TYPE::SR)
    {
        startSustain = static_cast<int>(t * sampleRate / 1000.f);
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

float EnvGen::getValue() const
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
        endStep = step + static_cast<int>(adsr.release * sampleRate / 1000.f);
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
        if (step >= startSustain && stage == ADSR_Stage::DECAY)
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
        if (startDecay != startAttack)
        {
            p = static_cast<float>(step - startAttack) / (startDecay - startAttack);
            value = interpolate(p, 0.0f, 1.0f, power);
            break;
        }
    case ADSR_Stage::DECAY:
        if (startSustain != startDecay)
        {
            p = static_cast<float>(step - startDecay) / (startSustain - startDecay);
            value = interpolate(p, 1.0f, adsr.sustain, power);
            break;
        }
    case ADSR_Stage::RELEASE:
        if (endStep != startRelease)
        {
            p = static_cast<float>(step - startRelease) / (endStep - startRelease);
            value = interpolate(p, adsr.sustain, 0.0f, power);
            break;
        }
    default:
        // value = 0.0f;
        break;
    }

    step++;
}

int EnvGen::getLength(float sustain_time = 0.0f) const
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

    return static_cast<int>(sampleRate * ms / 1000.0f);
}
