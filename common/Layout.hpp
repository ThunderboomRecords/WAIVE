#ifndef LAYOUT_HPP_INCLUDED
#define LAYOUT_HPP_INCLUDED

#include "NanoVG.hpp"
#include "Widget.hpp"

enum Widget_Align
{
    START = 0,
    CENTER,
    END
};

enum Position
{
    ON_TOP = 0,
    RIGHT_OF,
    LEFT_OF,
    ABOVE,
    BELOW
};

START_NAMESPACE_DISTRHO

namespace Layout
{
    void position(Position p, NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align h_align, Widget_Align v_align, float padding);

    void rightOf(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, float padding);

    void leftOf(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, float padding);

    void above(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, float padding);

    void below(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, float padding);

    void onTop(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align h_align, Widget_Align v_align, float h_padding, float v_padding);

    float measureVertical(NanoSubWidget *w1, Widget_Align a1, NanoSubWidget *w2, Widget_Align a2);

    float measureHorizontal(NanoSubWidget *w1, Widget_Align a1, NanoSubWidget *w2, Widget_Align a2);

}
END_NAMESPACE_DISTRHO

#endif
