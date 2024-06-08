#include "WidgetGroup.hpp"

START_NAMESPACE_DISTRHO

WidgetGroup::WidgetGroup(Widget *widget) noexcept
    : NanoSubWidget(widget)
{
}

void WidgetGroup::addChildWidget(NanoSubWidget *widget)
{
    children.push_back(widget);
}

void WidgetGroup::setVisible(bool visible)
{
    for (int i = 0; i < children.size(); i++)
        children[i]->setVisible(visible);
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

void WidgetGroup::onNanoDisplay() {}

END_NAMESPACE_DISTRHO