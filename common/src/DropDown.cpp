#include "DropDown.hpp"

DropDown::DropDown(Widget *parent) noexcept
    : NanoSubWidget(parent),
      font_size(12.0f),
      font(0),
      background_color(Color(200, 200, 200)),
      text_color(Color(30, 30, 30)),
      highlight_color(Color(180, 180, 180)),
      border_color(Color(120, 120, 120)),
      selected_item(0),
      menu(nullptr),
      callback(nullptr)
{
    loadSharedResources();
}

void DropDown::setFont(const char *name, const uchar *data, uint size)
{
    font = createFontFromMemory(name, data, size, false);
}

void DropDown::setDisplayNumber(int number)
{
    display_number = number;
}

void DropDown::onMenuItemSelection(Menu *menu, int item, const char *value)
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

    beginPath();
    fillColor(background_color);
    rect(0, 0, width, height);
    fill();
    closePath();

    beginPath();
    strokeColor(text_color);
    strokeWidth(1);
    rect(1, 1, width - 2, height - 2);
    stroke();
    closePath();

    beginPath();
    fillColor(text_color);
    moveTo(width - 5, height / 2 - 3);
    lineTo(width - 8, height / 2 + 3);
    lineTo(width - 11, height / 2 - 3);
    fill();
    closePath();

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
        std::cout << "DropDown::onMouse clicked" << std::endl;
        std::cout << items.size() << std::endl;

        menu->positionTo(this);
        menu->clear();
        for (int i = 0; i < items.size(); i++)
            menu->addItem(items[i].c_str());

        menu->calculateHeight();
        menu->setItem(selected_item, false);
        menu->font_size = font_size;
        menu->setCallback(this);
        menu->setDisplayNumber(display_number);
        menu->show();
        return true;
    }

    return false;
}

void DropDown::addItem(const char *item)
{
    items.push_back(item);
}

void DropDown::setItem(int item, bool sendCallback = false)
{
    currentItem.assign(items[item]);
    selected_item = item;
    if (sendCallback && callback != nullptr)
        callback->dropdownSelection(this, item);
}

void DropDown::setCallback(Callback *cb)
{
    callback = cb;
}