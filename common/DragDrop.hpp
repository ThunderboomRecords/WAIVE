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
    std::string info;
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
    class Callback
    {
    public:
        virtual ~Callback() {}
        virtual void onDragDropStart(DragDropWidget *widget, DragDropEvent &ev) = 0;
        virtual void onDragDropEnd(DragDropWidget *widget, DragDropEvent ev, bool accepted) = 0;
    };

    explicit DragDropManager(DGL::Window *window);
    void addWidget(DragDropWidget *widget);
    void addCallback(Callback *callback);

    void dragDropStart(DragDropWidget *widget, const std::string &data, const std::string &info = "");
    void dragDropEnd(DragDropWidget *widget, bool accepted);
    bool isDragging();
    void clearEvent();
    DragDropEvent getEvent();

private:
    bool hasEvent;
    std::shared_ptr<DragDropEvent> event;

    DGL::Window *window;

    std::vector<Callback *> callbacks;
};

END_NAMESPACE_DISTRHO

#endif