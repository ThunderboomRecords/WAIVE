#ifndef WAIVE_WIDGET_HPP_INCLUDED
#define WAIVE_WIDGET_HPP_INCLUDED

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include "fonts.h"

#include <iostream>

START_NAMESPACE_DISTRHO

class WAIVEWidget : public NanoSubWidget
{
public:
    explicit WAIVEWidget(Widget *widget, int flags = CREATE_ANTIALIAS) noexcept;

    /*
    GOTCHA: setSize() EXCLUDES scale factor, but getSize() INCLUDES scale factor
            so w1.setSize(w2.getSize()) will not necessarily make the widget the
            same size as the second
    TODO:   handle this? optional argument to ignore scaling factor? store "virtual"
            (i.e. unscaled) size info?
    */

    // Geometry
    void setSize(uint width, uint height);
    void setSize(const Size<uint> &size);

    // Drawing
    void fontSize(float size);

    // Color values
    Color background_color, foreground_color, highlight_color, stroke_color, accent_color;
    Color text_color;
    float font_size;
    FontId font;

protected:
    float scale_factor;
};

END_NAMESPACE_DISTRHO

#endif