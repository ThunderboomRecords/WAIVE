#ifndef H_BOX_HPP
#define H_BOX_HPP

#include <iostream>
#include <vector>

#include "WidgetGroup.hpp"

START_NAMESPACE_DISTRHO

class HBox : public WidgetGroup
{
public:
    explicit HBox(Widget *widget) noexcept;
    enum struct Align_Items
    {
        none,
        top,
        middle,
        bottom
    };

    enum struct Justify_Content
    {
        none,
        left,
        right,
        center,
        space_evenly,
        space_between
    };

    Align_Items align_items;
    Justify_Content justify_content;

    void addWidget(NanoSubWidget *widget);
    void setWidgetAlignment(uint id, Align_Items align_self);
    void setWidgetJustify_Content(uint id, Justify_Content justify_content);
    void removeWidget(uint id);

    void positionWidgets();
    void resizeToFit();
    int padding;

protected:
private:
    struct Item
    {
        Item(NanoSubWidget *w)
        {
            widget = w;
            width = 0;
            height = 0;
            align_self = Align_Items::none;
            justify_content = Justify_Content::none;
        }

        uint width;
        uint height;
        uint x;
        uint y;
        NanoSubWidget *widget;
        Align_Items align_self;
        Justify_Content justify_content;
    };

    Widget *parent_ = nullptr;

    std::vector<Item> items_;

    DISTRHO_LEAK_DETECTOR(HBox)
};

END_NAMESPACE_DISTRHO

#endif