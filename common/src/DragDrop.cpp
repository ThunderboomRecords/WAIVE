#include "DragDrop.hpp"

DragDropWidget::DragDropWidget(DragDropManager *manager)
{
    if (manager == nullptr)
        return;

    // manager->addWidget(this);
    dragDropManager = manager;
}

void DragDropWidget::dragStarted(DragDropEvent &ev)
{
    if (dragDropManager == nullptr)
        return;

    dragDropManager->dragDropStart(ev.source, ev.payload);
}

DragDropManager::DragDropManager(DGL::Window *window_)
    : event(nullptr),
      window(window_),
      hasEvent(false)
{
}

void DragDropManager::addWidget(DragDropWidget *widget)
{
    // widgets.push_back(std::make_shared<WAIVEWidget>(widget));
}

void DragDropManager::addCallback(Callback *callback)
{
    callbacks.push_back(callback);
}

void DragDropManager::dragDropStart(DragDropWidget *widget, const std::string &data, const std::string &info)
{
    std::cout << "DragDropManager::dragDropStart " << data << ", " << info << std::endl;
    DragDropEvent ev({widget, data, info});
    event = std::make_shared<DragDropEvent>(ev);
    hasEvent = true;

    for (auto &cb : callbacks)
        cb->onDragDropStart(widget, *event.get());

    window->setCursor(kMouseCursorHand);
}

bool DragDropManager::isDragging()
{
    // std::cout << "DragDropManager::isDragging() " << (event != nullptr) << std::endl;

    return hasEvent;
}

DragDropEvent DragDropManager::getEvent()
{
    return *event.get();
}

void DragDropManager::dragDropEnd(DragDropWidget *widget, bool accepted)
{
    std::cout << "DragDropManager::dragDropEnd " << accepted << std::endl;
    if (!hasEvent)
        return;

    for (auto &cb : callbacks)
        cb->onDragDropEnd(widget, getEvent(), true);

    clearEvent();
}

void DragDropManager::clearEvent()
{
    event = nullptr;
    hasEvent = false;
    window->setCursor(kMouseCursorArrow);
}