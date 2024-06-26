#include "DropDown.hpp"

DropDown::DropDown(Widget *parent) noexcept
    : WAIVEWidget(parent),
      selected_item(0),
      callback(nullptr)
{
    loadSharedResources();
    menu = new Menu(parent);
    menu->setCallback(this);
}

void DropDown::setDisplayNumber(int number)
{
    menu->setDisplayNumber(number);
}

void DropDown::onMenuItemSelection(Menu *menu, int item, const std::string &value)
{
    currentItem.assign(value);
    selected_item = item;
    if (callback != nullptr)
        callback->dropdownSelection(this, item);
    repaint();
}

void DropDown::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    if (renderDebug)
    {
        beginPath();
        strokeColor(accent_color);
        rect(0, 0, width, height);
        stroke();
        closePath();
    }

    if (currentItem.size() > 0)
    {
        beginPath();
        fillColor(text_color);
        fontFaceId(font);
        fontSize(font_size);
        textAlign(Align::ALIGN_MIDDLE);
        text(2, height / 2, currentItem.c_str(), nullptr);
        closePath();
    }
}

bool DropDown::onMouse(const MouseEvent &ev)
{
    if (!isVisible())
        return false;

    if (ev.press && ev.button == kMouseButtonLeft && menu != nullptr && contains(ev.pos))
    {

        menu->positionTo(this);
        menu->toFront();
        menu->show();
        return true;
    }

    return false;
}

bool DropDown::onMotion(const MotionEvent &ev)
{
    if (contains(ev.pos))
    {
        if (!hover)
        {
            getWindow().setCursor(kMouseCursorHand);
            hover = true;
        }
        return false;
    }
    else
    {
        if (hover)
        {
            getWindow().setCursor(kMouseCursorArrow);
            hover = false;
        }
        return false;
    }
    return false;
}

void DropDown::addItem(const char *item)
{
    menu->addItem(item);
}

void DropDown::setItem(int item, bool sendCallback = false)
{
    menu->setItem(item, sendCallback);
    currentItem.assign(menu->getItem(item));
    selected_item = item;
    if (sendCallback && callback != nullptr)
        callback->dropdownSelection(this, item);
}

void DropDown::setCallback(Callback *cb)
{
    callback = cb;
}