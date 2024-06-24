#ifndef WAVEFORM_HPP
#define WAVEFORM_HPP

#include <iostream>
#include <vector>

#include "WAIVEWidget.hpp"
#include "WaveformFeatures.hpp"

#ifndef LOG_LOCATION
#define LOG_LOCATION std::cout << __func__ << "():  " << __FILE__ << ":" << __LINE__ << std::endl;
#endif

START_NAMESPACE_DISTRHO

class Waveform : public WAIVEWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void waveformSelection(Waveform *waveform, uint selectionStart) = 0;
    };

    explicit Waveform(Widget *widget) noexcept;
    void setCallback(Callback *cb);
    void setWaveform(std::vector<float> *wf);
    void setWaveformFeatures(std::vector<WaveformFeature> *wfFeatures);
    void setWaveformLength(int length);
    void setSelection(int start, bool sendCallback);
    void waveformNew();
    void waveformUpdated();

    Color feature_color, cursor_color;
    bool selectable, zoomable;
    int level_of_detail;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;

private:
    enum DragAction
    {
        NONE = 0,
        CLICKING,
        SELECTING,
        SCROLLING,
    };

    void calculateWaveform();
    int getNearestFeature(float x);

    Callback *callback;
    std::vector<float> waveformMin, waveformMax;
    bool waveformCached, reduced;
    int waveformLength;
    int waveformSelectStart, waveformSelectEnd;

    DragAction dragAction;
    Point<double> clickStart;
    int visibleStart, visibleEnd;

    std::vector<float> *wf;
    std::vector<WaveformFeature> *wfFeatures;
    int featureHighlight;

    DISTRHO_LEAK_DETECTOR(Waveform);
};

END_NAMESPACE_DISTRHO

#endif