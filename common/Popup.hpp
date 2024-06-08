#ifndef POPUP_HPP_INCLUDED
#define POPUP_HPP_INCLUDED

#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

class Popup : WAIVEWidget
{
public:
    Popup(Widget *widget);

protected:
    void onNanoDisplay() override;
};

END_NAMESPACE_DISTRHO

#endif