#ifndef RADIO_BUTTONS_HPP_INCLUDED
#define RADIO_BUTTONS_HPP_INCLUDED

#include <string>
#include <iostream>

#include "Menu.hpp"

START_NAMESPACE_DISTRHO

class RadioButtons : public Menu
{
public:
    explicit RadioButtons(Widget *parent) noexcept;

    float radioSize;

    void calculateWidth();

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &ev) override;
    bool onMotion(const MotionEvent &ev) override;
    bool onKeyboard(const KeyboardEvent &ev) override;

private:
    DISTRHO_LEAK_DETECTOR(RadioButtons);
};

END_NAMESPACE_DISTRHO

#endif