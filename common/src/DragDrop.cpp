#include "DragDrop.hpp"

DragDropManager::DragDropManager()
{
}

void DragDropManager::addWidget(SubWidget *widget)
{
    widgets.push_back(widget);
}

void DragDropManager::dragDropStart(DragDropEvent &ev)
{
    event = &ev;
}

void DragDropManager::dragDropEnd(DragDropEvent &ev)
{
}