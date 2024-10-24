#include "Menu.hpp"

Menu::Menu(Widget *parent) noexcept
    : WAIVEWidget(parent),
      highlighted_item(-1),
      scroll_index(0),
      display_number(4),
      has_focus(false),
      alignment(Align::ALIGN_LEFT)
{
    hide();
}

void Menu::clear()
{
    items.clear();
    highlighted_item = -1;
    scroll_index = 0;
}

void Menu::addItem(const std::string &item)
{
    items.push_back(item);
    if (items.size() == 1)
        highlighted_item = 0;
}

void Menu::setDisplayNumber(int number)
{
    display_number = std::min(number, (int)items.size());
    calculateHeight();
}

void Menu::positionTo(NanoSubWidget *widget)
{
    int y = widget->getAbsoluteY();
    int maxY = getWindow().getHeight();

    if (y + getHeight() > maxY)
        y = maxY - getHeight();

    setAbsolutePos(widget->getAbsoluteX(), y);
    setWidth(widget->getWidth());
}

void Menu::onNanoDisplay()
{
    if (!isVisible())
        return;

    const float width = getWidth();
    const float height = getHeight();
    const float item_height = height / display_number;

    beginPath();
    fillColor(background_color);
    rect(0, 0, width, height);
    fill();
    closePath();

    beginPath();
    strokeColor(highlight_color);
    strokeWidth(2.0f);
    rect(0, 0, width, height);
    stroke();
    closePath();

    if (items.size() == 0)
        return;

    if (highlighted_item >= 0)
    {
        beginPath();
        fillColor(highlight_color);
        rect(0, (highlighted_item - scroll_index) * item_height, width, item_height);
        fill();
        closePath();
    }

    for (int i = 0; i < std::min(display_number, (int)items.size()); i++)
    {
        beginPath();
        fontSize(getFontSize());
        fontFaceId(font);
        fillColor(text_color);
        if (alignment == Align::ALIGN_LEFT)
        {
            textAlign(Align::ALIGN_LEFT | Align::ALIGN_TOP);
            text(2, i * item_height + 2, items[scroll_index + i].c_str(), NULL);
        }
        else
        {
            textAlign(Align::ALIGN_RIGHT | Align::ALIGN_TOP);
            text(width - 6, i * item_height + 2, items[scroll_index + i].c_str(), NULL);
        }
        closePath();
    }

    if (items.size() > display_number)
    {
        float steps = height / items.size();
        beginPath();
        fillColor(WaiveColors::light2);
        rect(width - 4, scroll_index * steps, 4, steps * display_number);
        fill();
        closePath();
    }
}

bool Menu::onMouse(const MouseEvent &ev)
{
    if (!isVisible() || items.size() == 0)
        return false;

    if (!contains(ev.pos))
    {
        hide();
        return false;
    }

    if (ev.press && ev.button == MouseButton::kMouseButtonLeft && has_focus)
    {
        if (highlighted_item < 0)
            return false;

        if (callback != nullptr)
            callback->onMenuItemSelection(this, highlighted_item, items[highlighted_item]);

        hide();
        getParentWidget()->repaint();
        return true;
    }

    return false;
}

bool Menu::onMotion(const MotionEvent &ev)
{
    if (!isVisible() || items.size() == 0)
        return false;

    bool hover = contains(ev.pos);

    if (!hover && has_focus)
    {
        hide(); // maybe not?
        has_focus = false;
        getWindow().repaint();

        return false;
    }
    else if (hover)
    {
        has_focus = true;
        const float item_height = getHeight() / display_number;
        highlighted_item = scroll_index + std::floor(ev.pos.getY() / item_height);

        repaint();
        return true;
    }

    return false;
}

bool Menu::onScroll(const ScrollEvent &ev)
{
    if (!isVisible() || items.size() == 0)
        return false;

    if (!contains(ev.pos))
        return false;

    int delta = 0;
    if (ev.delta.getY() >= 0.5f)
        delta = 1;
    else if (ev.delta.getY() < -0.5f)
        delta = -1;

    int max_scroll_index = std::max(0, (int)items.size() - display_number);
    scroll_index -= delta;
    scroll_index = std::clamp(scroll_index, 0, max_scroll_index);

    const float height = getHeight();
    const float item_height = height / display_number;
    highlighted_item = scroll_index + std::floor(ev.pos.getY() / item_height);

    repaint();
    return true;
}

bool Menu::onKeyboard(const KeyboardEvent &ev)
{
    if (!isVisible() || !ev.press)
        return false;

    switch (ev.key)
    {
    case kKeyEscape:
        has_focus = false;
        hide();
        getParentWidget()->repaint();
        return true;
        break;
    case kKeyEnter:
        if (callback != nullptr)
            callback->onMenuItemSelection(this, highlighted_item, items[highlighted_item]);
        hide();
        getParentWidget()->repaint();
        return true;
    case kKeyUp:
        if (highlighted_item > 0)
        {
            highlighted_item--;
            if (scroll_index > highlighted_item)
                scroll_index = std::min(highlighted_item, (int)items.size() - display_number);
        }
        repaint();
        return true;
    case kKeyDown:
        if (highlighted_item < items.size() - 1)
        {
            highlighted_item++;
            if (scroll_index + display_number <= highlighted_item)
                scroll_index = std::max(0, highlighted_item - display_number + 1);
        }
        repaint();
        return true;
    default:
        break;
    }

    return false;
}

void Menu::setItem(int item, bool sendCallback = false)
{
    highlighted_item = item;
    int max_scroll_index = std::max(0, (int)items.size() - display_number);
    scroll_index = std::clamp(highlighted_item, 0, max_scroll_index);
    if (callback != nullptr && sendCallback)
        callback->onMenuItemSelection(this, highlighted_item, items[highlighted_item]);
}

std::string Menu::getItem(int item) const
{
    try
    {
        return items.at(item);
    }
    catch (const std::exception &e)
    {
        return std::string();
    }
}

int Menu::getNumberItems() const
{
    return items.size();
}

void Menu::calculateHeight()
{
    Rectangle<float> bounds;
    fontSize(getFontSize());
    fontFaceId(font);
    textBounds(0, 0, "item 1", nullptr, bounds);

    setHeight(display_number * (bounds.getHeight() + 4));
}

void Menu::calculateWidth()
{
    Rectangle<float> bounds;
    float maxWidth = 10.f;
    fontSize(getFontSize());
    fontFaceId(font);
    for (size_t i = 0; i < items.size(); i++)
    {
        textBounds(0, 0, items.at(i).c_str(), nullptr, bounds);
        maxWidth = std::max(bounds.getWidth(), maxWidth);
    }

    setWidth(maxWidth + 6.f);
}

std::string Menu::getCurrentItem() const
{
    return getItem(highlighted_item);
}

void Menu::setCallback(Callback *cb)
{
    callback = cb;
}
