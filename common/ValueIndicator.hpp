#ifndef VALUE_INDICATOR_HPP_INCLUDED
#define VALUE_INDICATOR_HPP_INCLUDED

#include <iostream>
#include <fmt/core.h>

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"

START_NAMESPACE_DISTRHO

class ValueIndicator : public NanoSubWidget
{
public:
    explicit ValueIndicator(Widget *widget) noexcept;

    float font_size;
    Color background_color, text_color, border_color;
    void setFormatString(std::string fmt);
    void setValue(float val);
    FontId fontId;

protected:
    void onNanoDisplay() override;

private:
    std::string fFormat;
    float fValue;

};

END_NAMESPACE_DISTRHO

#endif