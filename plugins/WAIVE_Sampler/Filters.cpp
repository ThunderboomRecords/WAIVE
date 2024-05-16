#include "Filters.hpp"

Filter::Filter() : buf0(0.0),
                   buf1(0.0),
                   mode(FilterType::FILTER_LOWPASS),
                   cutoff(0.99),
                   resonance(0.0),
                   feedbackAmount(0.0)
{
}

void Filter::reset()
{
    buf0 = 0.0;
    buf1 = 0.0;
}

double Filter::process(double y)
{
    buf0 += cutoff * (y - buf0 + feedbackAmount * (buf0 - buf1));
    buf1 += cutoff * (buf0 - buf1);
    switch (mode)
    {
    case FilterType::FILTER_LOWPASS:
        return buf1;
    case FilterType::FILTER_HIGHPASS:
        return y - buf0;
    case FilterType::FILTER_BANDPASS:
        return buf0 - buf1;
    default:
        return 0.0;
    }
}

void Filter::setCutoff(double newCutoff)
{
    cutoff = std::clamp(newCutoff, 0.0, 0.999);
    calculateFeddbackAmount();
}

void Filter::setResonance(double newResonance)
{
    resonance = newResonance;
    calculateFeddbackAmount();
}

double Filter::getCutoff() const
{
    return cutoff;
}

double Filter::getResonance() const
{
    return resonance;
}

void Filter::calculateFeddbackAmount()
{
    feedbackAmount = resonance + resonance / (1.0 - cutoff);
}