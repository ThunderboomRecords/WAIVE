#ifndef DROPDOWN_HPP_INCLUDED
#define DROPDOWN_HPP_INCLUDED

#include "WAIVEWidget.hpp"

#include "Menu.hpp"

START_NAMESPACE_DISTRHO

class DropDown : public WAIVEWidget,
                 Menu::Callback
{
public:
    class Callback
    {
    public:
        virtual ~Callback() {};
        virtual void dropdownSelection(DropDown *widget, int item) = 0;
    };

    explicit DropDown(Widget *parent) noexcept;

    void setCallback(Callback *cb);
    void addItem(const char *item);
    void setDisplayNumber(int number);
    void setItem(int item, bool sendCallback);
    std::string getCurrentItem() const;

    Menu *menu;
    Align alignment;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &ev) override;
    bool onMotion(const MotionEvent &ev) override;
    void onMenuItemSelection(Menu *menu, int item, const std::string &value) override;

private:
    Callback *callback;

    std::string currentItem;
    // std::vector<std::string> items;
    int selected_item, display_number;
    bool hover;

    DISTRHO_LEAK_DETECTOR(DropDown);
};

END_NAMESPACE_DISTRHO

#endif