#include "CheckboxGroup.hpp"

START_NAMESPACE_DISTRHO

CheckboxGroup::CheckboxGroup(
    Widget *widget, float x, float y, float width, float height)
    : WidgetGroup(widget, x, y, width, height)
{
    check_all = new Button(widget);
    check_all->setLabel("all");
    check_all->setSize(30, 16);
    check_all->setCallback(this);
    check_all->onTop(this, Widget_Align::START, Widget_Align::START, 10);

    check_none = new Button(widget);
    check_none->setLabel("none");
    check_none->setSize(30, 16);
    check_none->setCallback(this);
    check_none->rightOf(check_all, Widget_Align::CENTER, 10);

    grid = new GridLayout(widget, x + 10, y + 30, width - 10, height - 50);

    addChildWidget(check_all);
    addChildWidget(check_none);
    addChildWidget(grid);
}

void CheckboxGroup::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(text_color);
    textAlign(Align::ALIGN_TOP | Align::ALIGN_CENTER);
    fontSize(getFontSize());
    text(width / 2.0f, 5.0f, label.c_str(), nullptr);
    closePath();
}

void CheckboxGroup::checkboxUpdated(Checkbox *checkbox, bool value)
{
    if (checkboxData.count(checkbox))
    {
        checkboxData[checkbox].checked = value;
        if (callback != nullptr)
            callback->checkboxesUpdated(this);
    }
}

void CheckboxGroup::addCheckbox(Checkbox *checkbox, const std::string &data)
{
    // Checkbox *checkbox = new Checkbox(getParentWidget());
    checkbox->setSize(100, 20);
    checkbox->setCallback(this);
    checkbox->label = data;
    grid->addChildWidget(checkbox);
    grid->positionWidgets();
    checkboxes.push_back(checkbox);
    checkboxData[checkbox] = {data, checkbox->getChecked()};
}

std::map<Checkbox *, CheckboxGroup::CheckboxData> CheckboxGroup::getCheckboxData()
{
    return checkboxData;
}

void CheckboxGroup::buttonClicked(Button *button)
{
    std::cout << "CheckboxGroup::buttonClicked\n";
    if (button == check_all)
    {
        checkAll(true, false);
        if (callback != nullptr)
            callback->checkboxesUpdated(this);
    }
    else if (button == check_none)
    {
        checkAll(false, false);
        if (callback != nullptr)
            callback->checkboxesUpdated(this);
    }
}

void CheckboxGroup::checkAll(bool check, bool sendCallback)
{
    std::cout << "checkAll: " << check << "\n";
    for (int i = 0; i < checkboxes.size(); i++)
    {
        checkboxes[i]->setChecked(check, sendCallback);
    }
}

void CheckboxGroup::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO