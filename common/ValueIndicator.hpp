#ifndef VALUE_INDICATOR_HPP_INCLUDED
#define VALUE_INDICATOR_HPP_INCLUDED

#include <iostream>
#include <fmt/core.h>

#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

class ValueIndicator : public WAIVEWidget
{
public:
    explicit ValueIndicator(Widget *widget) noexcept;

    void setFormatString(std::string fmt);
    void setValue(float val);

protected:
    void onNanoDisplay() override;

private:
    std::string fFormat;
    float fValue;

    DISTRHO_LEAK_DETECTOR(ValueIndicator);
};

END_NAMESPACE_DISTRHO

#endif