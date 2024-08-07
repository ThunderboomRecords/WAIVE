#ifndef CHECKBOX_GROUP_HPP_INCLUDED
#define CHECKBOX_GROUP_HPP_INCLUDED

#include <map>

#include "WidgetGroup.hpp"
#include "Checkbox.hpp"
#include "GridLayout.hpp"
#include "SimpleButton.hpp"

START_NAMESPACE_DISTRHO

class CheckboxGroup : public WidgetGroup,
                      Checkbox::Callback,
                      Button::Callback
{
public:
    struct CheckboxData
    {
        std::string data;
        bool checked;
    };

    class Callback
    {
    public:
        virtual ~Callback() {};
        virtual void checkboxesUpdated(CheckboxGroup *checkboxGroup) = 0;
    };
    CheckboxGroup(Widget *widget, float x, float y, float width, float height);
    void setCallback(Callback *cb);

    void addCheckbox(Checkbox *checkbox, const std::string &data);
    void checkAll(bool check, bool sendCallback);
    std::map<Checkbox *, CheckboxData> getCheckboxData();

    std::string label;

protected:
    void onNanoDisplay() override;
    void checkboxUpdated(Checkbox *checkbox, bool value) override;
    void buttonClicked(Button *button) override;

private:
    Callback *callback;
    std::vector<Checkbox *> checkboxes;
    std::map<Checkbox *, CheckboxData> checkboxData;

    GridLayout *grid;
    Button *check_all, *check_none;
};

END_NAMESPACE_DISTRHO

#endif