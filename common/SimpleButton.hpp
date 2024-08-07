#ifndef SIMPLEBUTTON_HPP_INCLUDED
#define SIMPLEBUTTON_HPP_INCLUDED

#include "WAIVEWidget.hpp"

#include <iostream>
#include <string>

START_NAMESPACE_DGL

class Button : public WAIVEWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback() {};
        virtual void buttonClicked(Button *button) = 0;
    };
    explicit Button(Widget *parent);

    void setCallback(Callback *cb);

    void setLabel(const std::string &label);
    void setEnabled(bool enabled);
    void setToggled(bool value, bool sendCallback = false);
    bool getToggled() const;
    void resizeToFit();

    bool drawBackground;
    bool isToggle;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &ev) override;
    bool onMotion(const MotionEvent &ev) override;

private:
    Callback *callback;

    std::string label;

    bool fHasFocus;
    bool fEnabled;
    bool fToggleValue;

    DISTRHO_LEAK_DETECTOR(Button)
};

END_NAMESPACE_DGL

#endif