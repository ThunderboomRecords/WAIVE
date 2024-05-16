#include "DropDown.hpp"

DropDown::DropDown(Widget *parent) noexcept
    : NanoSubWidget(parent),
      font_size(12.0f),
      background_color(Color(200, 200, 200)),
      text_color(Color(30, 30, 30)),
      highlight_color(Color(180, 180, 180)),
      border_color(Color(120, 120, 120))
{
    menu = new Menu(parent);
    menu->font_size = font_size;
    menu->background_color = background_color;
    menu->text_color = text_color;
    menu->border_color = border_color;
    menu->highlight_color = highlight_color;
    menu->hide();
    menu->setCallback(this);

    loadSharedResources();
}

void DropDown::setFont(const char *name, const uchar *data, uint size)
{
    font = createFontFromMemory(name, data, size, false);
    menu->setFont(name, data, size);
    repaint();
}

void DropDown::setDisplayNumber(int number)
{
    menu->setDisplayNumber(number);
}

void DropDown::onMenuItemSelection(Menu *menu, int item, const char *value)
{
    std::cout << "DropDown::onMenuItemSelection " << item << " " << value << std::endl;
    currentItem.assign(value);
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

    beginPath();
    fillColor(text_color);
    fontFaceId(font);
    fontSize(font_size);
    textAlign(Align::ALIGN_MIDDLE);
    text(2, height / 2, currentItem.c_str(), nullptr);
    closePath();
}

bool DropDown::onMouse(const MouseEvent &ev)
{
    if (!isVisible())
        return false;

    if (ev.press && ev.button == kMouseButtonLeft && contains(ev.pos))
    {
        menu->setWidth(getWidth());
        menu->font_size = font_size;
        menu->calculateHeight();
        menu->setAbsolutePos(getAbsolutePos());
        menu->show();
        return true;
    }

    return false;
}

void DropDown::addItem(const char *item)
{
    menu->addItem(item);
    if (menu->getNumberItems() == 1)
    {
        menu->setItem(0);
        currentItem.assign(menu->getItem(0));
    }
}

void DropDown::setItem(int item)
{
    menu->setItem(item);
    currentItem.assign(menu->getItem(item));
}

void DropDown::setCallback(Callback *cb)
{
    callback = cb;
}