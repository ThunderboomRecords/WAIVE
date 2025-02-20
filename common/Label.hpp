#ifndef LABEL_HPP_INCLUDED
#define LABEL_HPP_INCLUDED

#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

class Label : public WAIVEWidget
{
public:
    class Callback
    {
    public:
        ~Callback() {};
        virtual void labelClicked(Label *label) = 0;
    };

    explicit Label(Widget *widget, const std::string &text = "") noexcept;
    void setCallback(Callback *cb);

    void setLabel(const std::string &text);
    void resizeToFit();
    void calculateHeight();

    Align text_align;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &ev) override;

private:
    Callback *callback;
    std::string label;

    DISTRHO_LEAK_DETECTOR(Label);
};

END_NAMESPACE_DISTRHO

#endif
