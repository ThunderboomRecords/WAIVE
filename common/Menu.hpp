#ifndef MENU_HPP_INCLUDED
#define MENU_HPP_INCLUDED

#include <string>
#include <iostream>

#include "Widget.hpp"
#include "Window.hpp"
#include "NanoVG.hpp"

START_NAMESPACE_DISTRHO

class Menu : public NanoSubWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void onMenuItemSelection(Menu *menu, int item) = 0;
    };

    explicit Menu(Widget *parent) noexcept;

    void setFont(const char *name, const uchar *data, uint dataSize);
    void setCallback(Callback *cb);
    void addItem(const char *item);

    int display_number;

    Color background_color, text_color, highlight_color, border_color;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &ev) override;
    bool onMotion(const MotionEvent &ev) override;
    bool onScroll(const ScrollEvent &ev) override;

private:
    Callback *callback;

    FontId font;
    float font_size;
    std::vector<const char *> items;
    int highlighted_item;
    int scroll_index;

    DISTRHO_LEAK_DETECTOR(Menu);
};

END_NAMESPACE_DISTRHO

#endif