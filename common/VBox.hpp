#ifndef V_BOX_HPP
#define V_BOX_HPP

#include <iostream>
#include <vector>
#include <numeric>

#include "WidgetGroup.hpp"

START_NAMESPACE_DISTRHO

class VBox : public WidgetGroup
{
public:
    explicit VBox(Widget *widget) noexcept;
    enum struct Align_Items
    {
        none,
        left,
        middle,
        right
    };

    enum struct Justify_Content
    {
        none,
        top,
        bottom,
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
        Item(NanoSubWidget *w) : align_self(Align_Items::none),
                                 justify_content(Justify_Content::none),
                                 x(0),
                                 y(0),
                                 width(0),
                                 height(0)
        {
            widget = w;
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

    DISTRHO_LEAK_DETECTOR(VBox)
};

END_NAMESPACE_DISTRHO

#endif