#ifndef SAMPLE_MAP_HPP_INCLUDED
#define SAMPLE_MAP_HPP_INCLUDED

#include <map>
#include <iostream>

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"

#include "SampleDatabase.hpp"

START_NAMESPACE_DISTRHO

class SampleMap : public NanoSubWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void mapSampleSelected(int id) = 0;
    };

    explicit SampleMap(Widget *widget) noexcept;
    void setCallback(Callback *cb);

    Color background_color;

    std::vector<SampleInfo> *allSamples;
    SampleInfo *selectedSample;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;
    Color get2DColor(float x, float y);

private:
    Callback *callback;

    Color c0, c1, c2, c3;

    Point<double> embeddingToMap(Point<double>);
    Point<double> mapToEmbedding(Point<double>);

    float zoomLevel;
    Point<double> centerPos, dragStart, centerStart, cursorPos;

    std::map<int, Color> colorCache;

    bool dragging;
};

END_NAMESPACE_DISTRHO

#endif