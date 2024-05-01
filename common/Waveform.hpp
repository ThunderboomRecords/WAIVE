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
    class Callback {
        public:
            virtual ~Callback() {};
            virtual void waveformSelection(Waveform *waveform, uint selectionStart, uint selectionEnd) = 0;
    };

    explicit Waveform(Widget *widget) noexcept;
    void setCallback(Callback *cb);
    void calculateWaveform(std::vector<float> *wf);

    Color backgroundColor, lineColor;
    bool selectable;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;


private:
    Callback *callback;
    std::vector<float> waveformMin, waveformMax;
    bool waveformCached;
    uint waveformLength;
    uint waveformSelectStart;
    uint waveformSelectEnd;

    bool dragging;

    DISTRHO_LEAK_DETECTOR(Waveform);
};


END_NAMESPACE_DISTRHO

#endif