#ifndef WAIVE_WIDGET_HPP_INCLUDED
#define WAIVE_WIDGET_HPP_INCLUDED

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include "fonts.h"

#include "WAIVEColors.hpp"
#include "Layout.hpp"

#include <iostream>

START_NAMESPACE_DISTRHO

enum DragAction
{
    NONE = 0,
    CLICKING,
    SELECTING,
    SCROLLING,
};

class WAIVEWidget : public NanoSubWidget
{
public:
    class Callback
    {
        virtual ~Callback() {};
        virtual void onShowToolTip(WAIVEWidget *widget) = 0;
    };

    explicit WAIVEWidget(Widget *widget, int flags = CREATE_ANTIALIAS) noexcept;

    void below(NanoSubWidget *w, Widget_Align h_align = Widget_Align::CENTER, float padding = 10.f);
    void above(NanoSubWidget *w, Widget_Align h_align = Widget_Align::CENTER, float padding = 10.f);
    void leftOf(NanoSubWidget *w, Widget_Align v_align = Widget_Align::CENTER, float padding = 10.f);
    void rightOf(NanoSubWidget *w, Widget_Align v_align = Widget_Align::CENTER, float padding = 10.f);
    void onTop(NanoSubWidget *w, Widget_Align h_align = Widget_Align::CENTER, Widget_Align v_align = Widget_Align::CENTER, float h_padding = 10.f, float v_padding = -1.f);

    /*
    GOTCHA: setSize() EXCLUDES scale factor, but getSize() INCLUDES scale factor
            so w1.setSize(w2.getSize()) will not necessarily make the widget the
            same size as the second, unlese w1.setSize(..., true) is set
    */

    // Geometry
    void setSize(uint width, uint height, bool ignore_sf = false);
    void setSize(const Size<uint> &size, bool ignore_sf = false);

    // Drawing
    void setFont(const char *name, const uchar *data, uint size);

    // Properties
    void setFontSize(float size);
    float getFontSize(bool ignore_sf = false) const;
    float getTop() const;
    float getBottom() const;
    float getLeft() const;
    float getRight() const;
    float getCenterX() const;
    float getCenterY() const;
    void setLeft(float left);
    void setRight(float right);
    void setTop(float top);
    void setBottom(float bottom);
    void setCenterX(float centerX);
    void setCenterY(float centerY);

    // Color values
    Color background_color, foreground_color, highlight_color, accent_color;
    Color text_color;
    FontId font;

    std::string description;
    bool renderDebug;

protected:
    float scale_factor;

private:
    float font_size;
};

END_NAMESPACE_DISTRHO

#endif