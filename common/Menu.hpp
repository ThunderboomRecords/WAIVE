#ifndef MENU_HPP_INCLUDED
#define MENU_HPP_INCLUDED

#include <string>
#include <iostream>

#include "WAIVEWidget.hpp"

START_NAMESPACE_DISTRHO

class Menu : public WAIVEWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback() {};
        virtual void onMenuItemSelection(Menu *menu, int item, const std::string &value) = 0;
    };

    explicit Menu(Widget *parent) noexcept;

    void setCallback(Callback *cb);
    void addItem(const std::string &item);
    void clear();
    void setDisplayNumber(int number);
    void setItem(int item, bool sendCallback);
    int getNumberItems() const;
    std::string getItem(int item) const;
    std::string getCurrentItem() const;
    void calculateHeight();
    void calculateWidth();
    void positionTo(NanoSubWidget *widget);

    Align alignment;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &ev) override;
    bool onMotion(const MotionEvent &ev) override;
    bool onScroll(const ScrollEvent &ev) override;
    bool onKeyboard(const KeyboardEvent &ev) override;

private:
    Callback *callback;

    std::vector<std::string> items;
    int highlighted_item;
    int scroll_index;
    int display_number;

    bool has_focus;

    DISTRHO_LEAK_DETECTOR(Menu);
};

END_NAMESPACE_DISTRHO

#endif