#ifndef V_BOX_HPP
#define V_BOX_HPP

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include <vector>

START_NAMESPACE_DISTRHO

class VBox : public NanoSubWidget
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

    void addWidget(SubWidget *widget);
    void setWidgetAlignment(uint id, Align_Items align_self);
    void setWidgetJustify_Content(uint id, Justify_Content justify_content);
    void removeWidget(uint id);
    Color background_color;
    Color foreground_color;
    Color highlight_color;
    Color text_color;
    void positionWidgets();
    int padding;

protected:
    void onNanoDisplay() override;

private:
    struct Item
    {
        Item(SubWidget *w)
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
        SubWidget *widget;
        Align_Items align_self;
        Justify_Content justify_content;
    };

    Widget *parent_ = nullptr;

    std::vector<Item> items_;

    DISTRHO_LEAK_DETECTOR(VBox)
};

END_NAMESPACE_DISTRHO

#endif