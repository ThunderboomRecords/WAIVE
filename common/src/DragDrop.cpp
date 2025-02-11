#include "DragDrop.hpp"

DragDropWidget::DragDropWidget(DragDropManager *manager)
{
    if (manager == nullptr)
        return;

    manager->addWidget(this);
}

void DragDropWidget::dragStarted(DragDropEvent &ev)
{
    if (dragDropManager == nullptr)
        return;

    dragDropManager->dragDropStart(ev);
}

DragDropManager::DragDropManager()
{
}

void DragDropManager::addWidget(DragDropWidget *widget)
{
    widgets.push_back(std::make_shared<DragDropWidget>(widget));
}

void DragDropManager::dragDropStart(DragDropEvent &ev)
{
    event = std::make_unique<DragDropEvent>(ev);
}

bool DragDropManager::isDragging()
{
    return event != nullptr;
}

DragDropEvent DragDropManager::getEvent()
{
    return *event.get();
}

void DragDropManager::dragDropEnd(DragDropEvent &ev)
{
    clearEvent();
}

void DragDropManager::clearEvent()
{
    event = nullptr;
}