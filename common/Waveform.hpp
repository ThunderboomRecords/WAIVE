#ifndef WAVEFORM_HPP
#define WAVEFORM_HPP

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include <vector>
#include <iostream>

START_NAMESPACE_DISTRHO

enum DragAction
{
    NONE = 0,
    CLICKING,
    SELECTING,
    SCROLLING,
};

class Waveform : public NanoSubWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void waveformSelection(Waveform *waveform, uint selectionStart, uint selectionEnd) = 0;
    };

    explicit Waveform(Widget *widget) noexcept;
    void setCallback(Callback *cb);
    void setWaveform(std::vector<float> *wf);
    void setSelection(int start, int end, bool sendCallback);
    void waveformNew();
    void waveformUpdated();

    Color backgroundColor, lineColor;
    bool selectable, zoomable;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;

private:
    void calculateWaveform();

    Callback *callback;
    std::vector<float> waveformMin, waveformMax;
    bool waveformCached, reduced;
    uint waveformLength;
    int waveformSelectStart, waveformSelectEnd;

    DragAction dragAction;
    Point<double> clickStart;
    int visibleStart, visibleEnd;

    std::vector<float> *wf;

    DISTRHO_LEAK_DETECTOR(Waveform);
};

END_NAMESPACE_DISTRHO

#endif