#ifndef CHECKBOX_LIST_HPP_INCLUDED
#define CHECKBOX_LIST_HPP_INCLUDED

#include "WAIVEWidget.hpp"
#include "SimpleButton.hpp"

START_NAMESPACE_DISTRHO

class CheckboxList : public WAIVEWidget, Button::Callback
{
public:
    struct CheckboxData
    {
        std::string data;
        bool checked;
        Rectangle<double> rect;
    };

    class Callback
    {
    public:
        virtual ~Callback() {};
        virtual void checkboxesUpdated(CheckboxList *checkboxGroup) = 0;
    };

    CheckboxList(Widget *widget);

    void setCallback(Callback *cb);
    void reposition();
    void addItem(const std::string &name, bool checked);
    void clear();
    void checkAll(bool check, bool sendCallback);
    void setColumnCount(int n);
    std::vector<CheckboxData> *getData();

    float margin, padding;
    std::string label;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;
    void buttonClicked(Button *button) override;

private:
    void drawCheckbox(CheckboxData *data, float x, float y, float width, float height);
    std::vector<CheckboxData> data;

    Callback *callback;

    int columns;
    Button *check_all, *check_none;

    DISTRHO_LEAK_DETECTOR(CheckboxData);
};

END_NAMESPACE_DISTRHO

#endif