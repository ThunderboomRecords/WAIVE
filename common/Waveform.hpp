#ifndef WAVEFORM_HPP
#define WAVEFORM_HPP

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include <vector>
#include <iostream>

START_NAMESPACE_DISTRHO

class Waveform : public NanoSubWidget
{
public:
    explicit Waveform(Widget *widget) noexcept;
    void calculateWaveform(std::vector<float> *wf);

    Color backgroundColor, lineColor;
    bool *fSampleLoaded;

protected:
    void onNanoDisplay() override;


private:

    std::vector<float> waveformMin, waveformMax;
    bool waveformCached;

    DISTRHO_LEAK_DETECTOR(Waveform);
};


END_NAMESPACE_DISTRHO

#endif