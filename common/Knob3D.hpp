#ifndef KNOB_3D_HPP_INCLUDED
#define KNOB_3D_HPP_INCLUDED

#include <iostream>

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include "Knob.hpp"

START_NAMESPACE_DISTRHO

class Knob3D : public Knob
{
public:
    explicit Knob3D(Widget *widget) noexcept;

    void setKnobColor(Color color);

protected:
    void onNanoDisplay() override;

    Color line_color, knob_color;
    Paint inner_gradient, outer_gradient;

    DISTRHO_LEAK_DETECTOR(Knob3D);
};


END_NAMESPACE_DISTRHO

#endif