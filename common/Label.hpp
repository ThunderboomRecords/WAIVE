#ifndef LABEL_HPP_INCLUDED
#define LABEL_HPP_INCLUDED

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include <iostream>

START_NAMESPACE_DISTRHO

class Label : public NanoSubWidget
{
public:
    explicit Label(Widget *widget, std::string text) noexcept;

    void setText(std::string);
    void setFont(const char *name, const uchar *data, uint size);
    void resizeToFit();

    FontId font;
    float label_size;
    Color text_color;
    Align text_align;

protected:
    void onNanoDisplay() override;

private:
    std::string label;

    DISTRHO_LEAK_DETECTOR(Label);
};

END_NAMESPACE_DISTRHO

#endif
