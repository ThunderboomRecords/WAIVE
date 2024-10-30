#ifndef POPUP_HPP_INCLUDED
#define POPUP_HPP_INCLUDED

#include "WidgetGroup.hpp"
#include "SimpleButton.hpp"
#include "Panel.hpp"

START_NAMESPACE_DISTRHO

class Popup : public WidgetGroup,
              Button::Callback
{
public:
    class Callback
    {
    public:
        virtual ~Callback() {};
        virtual void popupOpened(Popup *popup) = 0;
        virtual void popupClosed(Popup *popup) = 0;
    };

    Popup(Widget *widget, float x, float y, float width, float height, bool ignore_sf = false);
    void setCallback(Callback *cb);

    float border_radius;

    void open();
    void close();

    std::string title;

protected:
    void onNanoDisplay() override;
    void buttonClicked(Button *button) override;

private:
    Button *close_btn;

    Callback *callback;

    DISTRHO_LEAK_DETECTOR(Popup);
};

END_NAMESPACE_DISTRHO

#endif