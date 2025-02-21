#include "GridLayout.hpp"

START_NAMESPACE_DISTRHO

GridLayout::GridLayout(Widget *widget, float width, float height, float x, float y) noexcept
    : WidgetGroup(widget, width, height, x, y),
      numColumns(1),
      margin(5.f),
      padding(10.f)
{
}

void GridLayout::positionWidgets()
{
    float width = getWidth();
    float colWidth = width / numColumns;

    for (int i = 0; i < children.size(); i++)
    {
        int col = i % numColumns;
        int row = i / numColumns;

        float x = padding + (colWidth + margin) * col;
        float y = padding + (rowHeight + margin) * row;

        children[i]->setAbsolutePos(x, y);
    }
}

END_NAMESPACE_DISTRHO