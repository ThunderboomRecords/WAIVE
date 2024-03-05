#ifndef XYSLIDER_HPP_INCLUDE
#define XYSLIDER_HPP_INCLUDE


#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include <iostream>

START_NAMESPACE_DISTRHO

class XYSlider : public NanoSubWidget
{
public:
    class Callback {
        public:
            virtual ~Callback() {};
            virtual void xyDragStarted(XYSlider *xySlider) = 0;
            virtual void xyDragFinished(XYSlider *xySlider, float x, float y) = 0;
            virtual void xyValueChanged(XYSlider *xySlider, float x, float y) = 0;
    };

    explicit XYSlider(Widget *parent) noexcept;

    void setCallback(Callback *cb);
    void setXValue(float x, bool sendCallback=false) noexcept;
    void setYValue(float y, bool sendCallback=false) noexcept;
    float getXValue() noexcept;
    float getYValue() noexcept;

    float minX, maxX, minY, maxY;
    Color fg_color, bg_color, marker_color;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;

private:
    void updateXY(const DGL::Point<double> pos);
    Callback *callback;
    bool dragging_;
    float value_x_, value_y_;
    
    DISTRHO_LEAK_DETECTOR(XYSlider);
};


END_NAMESPACE_DISTRHO
#endif