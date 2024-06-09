#ifndef SPINNER_HPP_INCLUDED
#define SPINNER_HPP_INCLUDED

#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

class Spinner : public WAIVEWidget, public IdleCallback
{
public:
    explicit Spinner(Widget *widget) noexcept;

    void setLoading(bool l);
    bool getLoading() const;
    void idleCallback() override;

    Color foreground_color;

protected:
    void onNanoDisplay() override;

private:
    bool loading;
    float angle;
};

END_NAMESPACE_DISTRHO

#endif