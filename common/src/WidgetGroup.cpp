#include "WidgetGroup.hpp"

START_NAMESPACE_DISTRHO

WidgetGroup::WidgetGroup(Widget *widget, float x, float y, float width, float height) noexcept
    : WAIVEWidget(widget)
{
    setSize(width, height);
    setAbsolutePos(x, y);
}

void WidgetGroup::addChildWidget(NanoSubWidget *widget)
{
    children.push_back(widget);
}

void WidgetGroup::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    rect(0, 0, width, height);
    strokeColor(stroke_color);
    stroke();
    closePath();
}

void WidgetGroup::setVisible(bool visible)
{
    std::cout << "WidgetGroup::setVisible visible: " << visible << " numChildren: " << children.size() << "\n";
    for (int i = 0; i < children.size(); i++)
    {
        children[i]->setVisible(visible);
        children[i]->setSkipDrawing(!visible);
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