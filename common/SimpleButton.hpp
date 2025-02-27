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
        virtual void buttonDragged(Button *button) {};
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
    enum DragAction
    {
        NONE,
        CLICKING,
        DRAGGING
    };

    Callback *callback;

    std::string label;

    bool fHasFocus;
    bool fEnabled;
    bool fToggleValue;

    DragAction fDragAction;
    DGL::Point<double> fDragStartPos;

    DISTRHO_LEAK_DETECTOR(Button)
};

END_NAMESPACE_DGL

#endif