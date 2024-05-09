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

    std::vector<std::shared_ptr<SampleInfo>> *allSamples;
    std::shared_ptr<SampleInfo> *selectedSample;
    int highlightSample;

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

    Color get2DColor(float x, float y);
    Callback *callback;

    Color c0, c1, c2, c3;

    Point<double> embeddingToMap(Point<double>);
    Point<double> mapToEmbedding(Point<double>);

    float zoomLevel;
    Point<double> centerPos, dragStart, centerStart, cursorPos;

    std::map<int, Color> colorCache;

    // bool dragging;
    DragAction dragAction;
};

END_NAMESPACE_DISTRHO

#endif