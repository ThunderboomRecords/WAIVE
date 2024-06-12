#ifndef WIDGET_GROUP_HPP_INCLUDED
#define WIDGET_GROUP_HPP_INCLUDED

#include <iostream>
#include <vector>

#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

class WidgetGroup : public WAIVEWidget
{
public:
    explicit WidgetGroup(Widget *widget, float x = 100.f, float y = 100.f, float width = 100.f, float height = 120.f) noexcept;

    void addChildWidget(NanoSubWidget *widget);
    void setVisible(bool visible);
    void toFront();
    void toBottom();

protected:
    void onNanoDisplay() override;
    std::vector<NanoSubWidget *> children;
};

END_NAMESPACE_DISTRHO

#endif