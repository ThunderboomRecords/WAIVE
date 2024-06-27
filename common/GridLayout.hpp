#ifndef GRID_LAYOUT_HPP_INCLUDED
#define GRID_LAYOUT_HPP_INCLUDED

#include "WidgetGroup.hpp"

START_NAMESPACE_DISTRHO

class GridLayout : public WidgetGroup
{
public:
    explicit GridLayout(Widget *widget, float width, float height, float x, float y) noexcept;

    void positionWidgets();
    void resizeToFit();

    int numColumns;
    float margin, padding;
    float rowHeight;
};

END_NAMESPACE_DISTRHO

#endif