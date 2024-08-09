#ifndef WIDGET_GROUP_HPP_INCLUDED
#define WIDGET_GROUP_HPP_INCLUDED

#include <iostream>
#include <vector>

#include "WAIVEWidget.hpp"
#include "Layout.hpp"

START_NAMESPACE_DISTRHO

class WidgetGroup : public WAIVEWidget
{
public:
    struct WidgetPosition
    {
        NanoSubWidget *widget;
        NanoSubWidget *target;
        Position p = Position::ON_TOP;
        Widget_Align hAlign = Widget_Align::CENTER;
        Widget_Align vAlign = Widget_Align::CENTER;
        int padding;
    };

    explicit WidgetGroup(Widget *widget, float x = 100.f, float y = 100.f, float width = 100.f, float height = 120.f, bool ignore_sf = false) noexcept;

    void addChildWidget(NanoSubWidget *widget);
    void addChildWidget(NanoSubWidget *widget, WidgetPosition position);
    void setVisible(bool visible);
    void toFront() override;
    void toBottom() override;

    void computeSize();
    void repositionWidgets();

protected:
    void onNanoDisplay() override;
    std::vector<NanoSubWidget *> children;
    std::vector<WidgetPosition> positions;
};

END_NAMESPACE_DISTRHO

#endif