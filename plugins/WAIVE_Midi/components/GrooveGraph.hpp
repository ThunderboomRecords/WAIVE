#ifndef GROOVEGRAPH_HPP_INCLUDED
#define GROOVEGRAPH_HPP_INCLUDED

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include "Notes.hpp"

START_NAMESPACE_DISTRHO


class GrooveGraph : public NanoSubWidget
{
public:
    class Callback {
        public:
            virtual ~Callback() {};
            virtual void grooveClicked(GrooveGraph *graph) = 0;
    };  
    explicit GrooveGraph(Widget *widget) noexcept;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;

    int nearestElementAtPos(float position);

private:
    Callback *callback;
    std::vector<GrooveEvent> *fGroove;

    bool dragging;
    Point<double> dragStart;
    
    DISTRHO_LEAK_DETECTOR(GrooveGraph);

    friend class WAIVEMidiUI;
};


END_NAMESPACE_DISTRHO

#endif