#ifndef DROPDOWN_HPP_INCLUDED
#define DROPDOWN_HPP_INCLUDED

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include <iostream>

#include "Menu.hpp"

START_NAMESPACE_DISTRHO

class DropDown : public NanoSubWidget,
                 public Menu::Callback
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void dropdownSelection(DropDown *widget, int item) = 0;
    };

    explicit DropDown(Widget *parent) noexcept;

    void setFont(const char *name, const uchar *data, uint dataSize);
    void setCallback(Callback *cb);
    void addItem(const char *item);
    void setDisplayNumber(int number);
    void setItem(int item);

    Menu *menu;
    Color background_color, text_color, highlight_color, border_color;
    float font_size;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &ev) override;
    void onMenuItemSelection(Menu *menu, int item, const char *value) override;

private:
    Callback *callback;

    FontId font;

    std::string currentItem;

    DISTRHO_LEAK_DETECTOR(DropDown);
};

END_NAMESPACE_DISTRHO

#endif