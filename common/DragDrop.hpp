#ifndef DRAG_DROP_HPP_INCLUDED
#define DRAG_DROP_HPP_INCLUDED

#include <string>
#include <vector>
#include <memory>

#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

class DragDropManager;
class DragDropWidget;
struct DragDropEvent
{
    DragDropWidget *source;
    std::string payload;
};

class DragDropWidget
{
public:
    explicit DragDropWidget(DragDropManager *manager);

    virtual void dataAccepted(DragDropWidget *destination) = 0;
    virtual void dataRejected(DragDropWidget *destination) = 0;

protected:
    DragDropManager *dragDropManager;

private:
    void dragStarted(DragDropEvent &ev);
};

class DragDropManager
{
public:
    explicit DragDropManager(DGL::Window *window);
    void addWidget(DragDropWidget *widget);

    void dragDropStart(DragDropWidget *widget, const std::string &data);
    void dragDropEnd(DragDropEvent &ev);
    bool isDragging();
    void clearEvent();
    DragDropEvent getEvent();

private:
    // std::vector<std::shared_ptr<DragDropWidget>> widgets;
    bool hasEvent;
    std::shared_ptr<DragDropEvent> event;

    DGL::Window *window;
};

END_NAMESPACE_DISTRHO

#endif