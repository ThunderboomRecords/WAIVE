#include "CheckboxList.hpp"

START_NAMESPACE_DISTRHO

CheckboxList::CheckboxList(Widget *widget)
    : WAIVEWidget(widget),
      columns(2),
      margin(5.0f),
      padding(5.0f),
      label("")
{
    check_all = new Button(widget);
    check_all->setLabel("all");
    check_all->setSize(40, 16);
    check_all->setCallback(this);

    check_none = new Button(widget);
    check_none->setLabel("none");
    check_none->setSize(40, 16);
    check_none->setCallback(this);
}

void CheckboxList::addItem(const std::string &name, bool checked = false)
{
    data.push_back({std::string(name), checked});
}

std::vector<CheckboxList::CheckboxData> *CheckboxList::getData()
{
    return &data;
}

void CheckboxList::clear()
{
    data.clear();
}

void CheckboxList::setColumnCount(int c)
{
    columns = c;
    repaint();
}

void CheckboxList::buttonClicked(Button *button)
{
    if (button == check_all)
        checkAll(true, true);
    else if (button == check_none)
        checkAll(false, true);
}

void CheckboxList::checkAll(bool check, bool sendCallback)
{
    for (int i = 0; i < data.size(); i++)
        data[i].checked = check;

    repaint();

    if (sendCallback && callback != nullptr)
        callback->checkboxesUpdated(this);
}

void CheckboxList::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    float colWidth = (width - 2 * padding - (columns - 1) * margin) / columns;
    float rowHeight = 1.5f * getFontSize();

    beginPath();
    fillColor(background_color);
    rect(0, 0, width, height);
    fill();
    stroke();
    closePath();

    beginPath();
    fillColor(text_color);
    textAlign(Align::ALIGN_TOP | Align::ALIGN_RIGHT);
    text(width - 10, 10, label.c_str(), nullptr);
    closePath();

    int col = 0;
    int row = 0;

    float x, y;

    for (int i = 0; i < data.size(); ++i)
    {
        col = i % columns;
        row = i / columns;

        x = padding + (colWidth + margin) * col;
        y = 36 + padding + (rowHeight + margin) * row;

        drawCheckbox(&data[i], x, y, colWidth, rowHeight);
    }
}

void CheckboxList::drawCheckbox(CheckboxData *data, float x, float y, float width, float height)
{
    float r = getFontSize() / 2.0f;

    data->rect.setRectangle({x, y}, {width, height});

    translate(x, y);

    beginPath();
    strokeColor(data->checked ? accent_color : foreground_color);
    circle(r, height / 2, r - 2.f);
    stroke();
    closePath();

    beginPath();
    fillColor(data->checked ? accent_color : foreground_color);
    circle(r, height / 2, r - 5.f);
    fill();
    closePath();

    // label:
    beginPath();
    fillColor(text_color);
    fontSize(getFontSize());
    textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_LEFT);
    text(2 * r + 4.0f, height / 2.0f, data->data.c_str(), nullptr);
    closePath();

    resetTransform();
}

bool CheckboxList::onMouse(const MouseEvent &ev)
{
    if (!ev.press || ev.button != kMouseButtonLeft || !contains(ev.pos))
        return false;

    bool found = false;
    for (int i = 0; i < data.size(); i++)
    {
        if (data[i].rect.contains(ev.pos))
        {
            data[i].checked = !data[i].checked;
            found = true;
            if (callback != nullptr)
                callback->checkboxesUpdated(this);
            repaint();
            break;
        }
    }

    return found;
}

bool CheckboxList::onMotion(const MotionEvent &ev)
{
    return false;
}

bool CheckboxList::onScroll(const ScrollEvent &ev)
{
    return false;
}

void CheckboxList::setCallback(Callback *cb)
{
    callback = cb;
}

void CheckboxList::reposition()
{
    check_all->onTop(this, Widget_Align::START, Widget_Align::START, 10);
    check_none->rightOf(check_all, Widget_Align::CENTER, 10);
}

END_NAMESPACE_DISTRHO