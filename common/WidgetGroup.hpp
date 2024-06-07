#ifndef WIDGET_GROUP_HPP_INCLUDED
#define WIDGET_GROUP_HPP_INCLUDED

#include <iostream>
#include <vector>

#include "NanoVG.hpp"

START_NAMESPACE_DISTRHO

class WidgetGroup : public NanoSubWidget
{
public:
    explicit WidgetGroup(Widget *widget) noexcept;

    void addChildWidget(NanoSubWidget *widget);
    void setVisible(bool visible);

protected:
    void onNanoDisplay() override;

private:
    std::vector<NanoSubWidget *> children;
};

END_NAMESPACE_DISTRHO

#endif