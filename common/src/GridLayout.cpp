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

    int col = 0;
    int row = 0;

    float x, y;

    for (int i = 0; i < children.size(); i++)
    {
        col = i % numColumns;
        row = i / numColumns;

        x = padding + (colWidth + margin) * col;
        y = padding + (rowHeight + margin) * row;

        children[i]->setAbsolutePos(x, y);
    }
}

END_NAMESPACE_DISTRHO