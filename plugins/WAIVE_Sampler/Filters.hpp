#ifndef FILTERS_HPP_INCLUDED
#define FILTERS_HPP_INCLUDED

#include <algorithm>

class Filter
{
public:
    enum FilterType
    {
        FILTER_LOWPASS = 0,
        FILTER_HIGHPASS,
        FILTER_BANDPASS
    };

    Filter();

    void reset();
    double process(double y);
    void setCutoff(double cutoff);
    double getCutoff() const;
    void setResonance(double resonance);
    double getResonance() const;

    FilterType mode;

private:
    void calculateFeddbackAmount();

    double cutoff;
    double resonance;
    double buf0, buf1;
    double feedbackAmount;
};

#endif
