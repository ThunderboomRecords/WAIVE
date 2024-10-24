#include "DropDown.hpp"

DropDown::DropDown(Widget *parent) noexcept
    : WAIVEWidget(parent),
      selected_item(0),
      callback(nullptr),
      hover(false),
      alignment(Align::ALIGN_LEFT)
{
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

    if (hover)
    {
        beginPath();
        strokeColor(highlight_color);
        strokeWidth(2.0f);
        rect(0, 0, width, height);
        stroke();
        closePath();
    }

    if (currentItem.size() > 0)
    {
        beginPath();
        fillColor(text_color);
        fontFaceId(font);
        fontSize(getFontSize());
        if (alignment == Align::ALIGN_LEFT)
        {
            textAlign(Align::ALIGN_LEFT | Align::ALIGN_MIDDLE);
            text(2, height / 2, currentItem.c_str(), nullptr);
        }
        else
        {
            textAlign(Align::ALIGN_RIGHT | Align::ALIGN_MIDDLE);
            text(width - 6, height / 2, currentItem.c_str(), nullptr);
        }
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
            repaint();
        }
        return false;
    }
    else
    {
        if (hover)
        {
            getWindow().setCursor(kMouseCursorArrow);
            hover = false;
            repaint();
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

std::string DropDown::getCurrentItem() const
{
    return menu->getCurrentItem();
}

void DropDown::setCallback(Callback *cb)
{
    callback = cb;
}