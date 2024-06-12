#ifndef PANEL_HPP_INCLUDED
#define PANEL_HPP_INCLUDED

#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

class Panel : public WAIVEWidget
{
public:
    Panel(Widget *widget);

protected:
    void onNanoDisplay() override;
    bool onMotion(const MotionEvent &ev) override;
    bool onMouse(const MouseEvent &ev) override;
    bool onScroll(const ScrollEvent &ev) override;
};

END_NAMESPACE_DISTRHO

#endif