#ifndef SPINNER_HPP_INCLUDED
#define SPINNER_HPP_INCLUDED

#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

class Spinner : public WAIVEWidget, public IdleCallback
{
public:
    explicit Spinner(Widget *widget) noexcept;

    void idleCallback() override;

    Color foreground_color;

protected:
    void onNanoDisplay() override;

private:
    float angle;
};

END_NAMESPACE_DISTRHO

#endif