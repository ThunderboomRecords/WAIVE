#ifndef DRAG_DROP_HPP_INCLUDED
#define DRAG_DROP_HPP_INCLUDED

#include <string>
#include <vector>
#include <memory>

#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

struct DragDropEvent
{
    std::shared_ptr<WAIVEWidget> source;
    std::string payload;
};

class DragDropManager;

class DragDropWidget
{
public:
    explicit DragDropWidget(DragDropManager *manager);
    ~DragDropWidget() {};

protected:
    DragDropManager *dragDropManager;

private:
    void dragStarted(DragDropEvent &ev);
};

class DragDropManager
{
public:
    explicit DragDropManager();
    void addWidget(DragDropWidget *widget);

    void dragDropStart(DragDropEvent &ev);
    void dragDropEnd(DragDropEvent &ev);
    bool isDragging();
    void clearEvent();
    DragDropEvent getEvent();

private:
    std::vector<std::shared_ptr<DragDropWidget>> widgets;
    std::unique_ptr<DragDropEvent> event;
};

END_NAMESPACE_DISTRHO

#endif