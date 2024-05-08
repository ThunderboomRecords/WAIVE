#ifndef LAYOUT_HPP_INCLUDED
#define LAYOUT_HPP_INCLUDED

#include "NanoVG.hpp"
#include "Widget.hpp"

enum class Widget_Align
{
    START = 0,
    CENTER,
    END
};

START_NAMESPACE_DISTRHO

namespace Layout
{

    void rightOf(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, int padding);

    void leftOf(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, int padding);

    void above(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, int padding);

    void below(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align align, int padding);

    void onTop(NanoSubWidget *w1, NanoSubWidget *w2, Widget_Align h_align, Widget_Align v_align, int padding);

}
END_NAMESPACE_DISTRHO

#endif
