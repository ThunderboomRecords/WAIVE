#ifndef POPUP_HPP_INCLUDED
#define POPUP_HPP_INCLUDED

#include "WidgetGroup.hpp"
#include "SimpleButton.hpp"
#include "Layout.hpp"

START_NAMESPACE_DISTRHO

class Popup : public WidgetGroup,
              Button::Callback
{
public:
    Popup(Widget *widget);

    Color background_color, border_color;
    float border_radius;

    void open();
    void close();

protected:
    void onNanoDisplay() override;
    void buttonClicked(Button *button) override;

    bool onMouse(const MouseEvent &ev) override;

private:
    Button *close_btn;
};

END_NAMESPACE_DISTRHO

#endif