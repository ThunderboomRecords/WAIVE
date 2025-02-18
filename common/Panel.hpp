#ifndef PANEL_HPP_INCLUDED
#define PANEL_HPP_INCLUDED

#include "WidgetGroup.hpp"

START_NAMESPACE_DISTRHO

class Panel : public WidgetGroup
{
public:
    Panel(Widget *widget);

    std::string title;
    std::string label;

    float padding_h, padding_v, expand_h, expand_v;
    uint size_w, size_h;
    float radius;
    bool expandable, expanded, expand_right, expand_down;

    WidgetGroup hiddenWidgets;

    void setSize(uint width, uint height, bool ignore_sf = false);
    void setSize(const Size<uint> &size, bool ignore_sf = false);
    void getTitlAbsoluteBounds(Rectangle<float> &bounds);
    void expand();
    void collapse();
    void toggle();

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &ev) override;
    bool onMotion(const MotionEvent &ev) override;

    DISTRHO_LEAK_DETECTOR(Panel);
};

END_NAMESPACE_DISTRHO

#endif