#include "WidgetGroup.hpp"

START_NAMESPACE_DISTRHO

WidgetGroup::WidgetGroup(Widget *widget, float x, float y, float width, float height, bool ignore_sf) noexcept
    : WAIVEWidget(widget)
{
    setSize(width, height, ignore_sf);
    setAbsolutePos(x, y);
}

void WidgetGroup::addChildWidget(NanoSubWidget *widget)
{
    children.push_back(widget);
}

void WidgetGroup::addChildWidget(NanoSubWidget *widget, WidgetPosition position)
{
    children.push_back(widget);
    positions.push_back(position);
}

void WidgetGroup::onNanoDisplay()
{
    if (!renderDebug)
        return;

    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    rect(0, 0, width, height);
    strokeColor(accent_color);
    stroke();
    closePath();
}

void WidgetGroup::computeSize()
{
    // Resizes itself to become bounding box of all child elements
    int x1, y1, x2, y2;
    x1 = 100000;
    x2 = -100000;
    y1 = 100000;
    y2 = -100000;

    for (int i = 0; i < children.size(); i++)
    {
        Rectangle<int> wBounds = children[i]->getAbsoluteArea();
        x1 = std::min(x1, wBounds.getX());
        y1 = std::min(y1, wBounds.getY());
        x2 = std::max(x2, wBounds.getX() + wBounds.getWidth());
        y2 = std::max(y2, wBounds.getY() + wBounds.getHeight());
    }

    setAbsolutePos(x1, y1);
    setSize(x2 - x1, y2 - y1);
}

void WidgetGroup::repositionWidgets()
{
    for (int i = 0; i < positions.size(); i++)
    {
        Layout::position(
            positions[i].p,
            positions[i].widget,
            positions[i].target,
            positions[i].hAlign,
            positions[i].vAlign,
            positions[i].padding);
    }
}

void WidgetGroup::setVisible(bool visible)
{
    for (int i = 0; i < children.size(); i++)
    {
        // check if WidgetGroup
        WidgetGroup *group = dynamic_cast<WidgetGroup *>(children.at(i));
        if (group == nullptr)
            children[i]->setVisible(visible);
        else
            group->setVisible(visible);
    }
}

void WidgetGroup::toFront()
{
    for (int i = 0; i < children.size(); i++)
        children[i]->toFront();
}

void WidgetGroup::toBottom()
{
    for (int i = 0; i < children.size(); i++)
        children[i]->toBottom();
}

END_NAMESPACE_DISTRHO