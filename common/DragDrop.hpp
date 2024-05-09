#ifndef DRAG_DROP_HPP_INCLUDED
#define DRAG_DROP_HPP_INCLUDED

#include <string>
#include <vector>

#include "Widget.hpp"

START_NAMESPACE_DISTRHO

struct DragDropEvent
{
    int id;
    std::string payload;
};

class DragDropManager
{
public:
    explicit DragDropManager();
    void addWidget(SubWidget *widget);

    void dragDropStart(DragDropEvent &ev);
    void dragDropEnd(DragDropEvent &ev);

private:
    std::vector<SubWidget *> widgets;
    DragDropEvent *event;
};

END_NAMESPACE_DISTRHO

#endif