#ifndef SLIDER_HPP_INCLUDED
#define SLIDER_HPP_INCLUDED

#include "Widget.hpp"
#include "NanoVG.hpp"

START_NAMESPACE_DISTRHO

class Slider : public NanoSubWidget
{
public:
    class Callback {
        public:
            virtual ~Callback() {};
            virtual void sliderDragStarted(Slider *slider) = 0;
            virtual void sliderDragFinished(Slider *slider, float value) = 0;
            virtual void sliderValueChanged(Slider *slider, float value) = 0;
    };

    explicit Slider(Widget *widget) noexcept;
};

END_NAMESPACE_DISTRHO

#endif