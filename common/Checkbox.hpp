#ifndef CHECKBOX_HPP_INCLUDED
#define CHECKBOX_HPP_INCLUDED

#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

class Checkbox : public WAIVEWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void checkboxUpdated(Checkbox *checkbox, bool value) = 0;
    };

    explicit Checkbox(Widget *parent);

    void setCallback(Callback *cb);
    void setChecked(bool checked, bool sendCallback);
    bool getChecked() const;
    void resize();

    std::string label;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &ev) override;
    bool onMotion(const MotionEvent &ev) override;

private:
    Callback *callback;

    bool checked, focused;

    DISTRHO_LEAK_DETECTOR(Checkbox);
};

END_NAMESPACE_DISTRHO

#endif