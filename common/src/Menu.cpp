#include "Menu.hpp"

Menu::Menu(Widget *parent) noexcept
    : NanoSubWidget(parent),
      highlighted_item(-1),
      scroll_index(0),
      display_number(4),
      font_size(12.0f),
      background_color(Color(200, 200, 200)),
      text_color(Color(30, 30, 30)),
      highlight_color(Color(180, 180, 180)),
      border_color(Color(120, 120, 120)),
      has_focus(false)
{
    loadSharedResources();
}

void Menu::setFont(const char *name, const uchar *data, uint size)
{
    font = createFontFromMemory(name, data, size, false);
    repaint();
}

void Menu::clear()
{
    items.clear();
    highlighted_item = -1;
    scroll_index = 0;
}

void Menu::addItem(const char *item)
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
    // TODO: make sure it fits fully in parent window
    setAbsolutePos(widget->getAbsolutePos());
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

    beginPath();
    strokeColor(border_color);
    strokeWidth(1);
    rect(1, 1, width - 2, height - 2);
    stroke();
    closePath();

    for (int i = 0; i < std::min(display_number, (int)items.size()); i++)
    {
        beginPath();
        fontSize(font_size);
        fontFaceId(font);
        fillColor(text_color);
        textAlign(Align::ALIGN_LEFT | Align::ALIGN_TOP);
        text(2, i * item_height + 2, items[scroll_index + i], nullptr);
        closePath();
    }

    if (items.size() > display_number)
    {
        float steps = height / items.size();
        beginPath();
        fillColor(border_color);
        rect(width - 4, scroll_index * steps, 4, steps * display_number);
        fill();
        closePath();
    }
}

bool Menu::onMouse(const MouseEvent &ev)
{
    if (!isVisible() || items.size() == 0)
        return false;

    if (ev.press && ev.button == MouseButton::kMouseButtonLeft && has_focus)
    {
        if (!contains(ev.pos))
        {
            hide();
            repaint();
            return false;
        }

        if (highlighted_item < 0)
            return false;

        if (callback != nullptr)
            callback->onMenuItemSelection(this, highlighted_item, items[highlighted_item]);

        hide();
        repaint();
        return true;
    }

    return false;
}

bool Menu::onMotion(const MotionEvent &ev)
{
    if (!isVisible() || items.size() == 0)
        return false;

    if (!contains(ev.pos) && has_focus)
    {
        hide(); // maybe not?
        repaint();
        has_focus = false;
        return false;
    }
    else if (contains(ev.pos))
    {
        has_focus = true;
        const float height = getHeight();
        const float item_height = height / display_number;
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
    {
        hide();
        return false;
    }

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
        return true;
        break;
    case kKeyEnter:
        if (callback != nullptr)
            callback->onMenuItemSelection(this, highlighted_item, items[highlighted_item]);
        hide();
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

const char *Menu::getItem(int item) const
{
    try
    {
        return items.at(item);
    }
    catch (const std::exception &e)
    {
        return "";
    }
}

int Menu::getNumberItems() const
{
    return items.size();
}

void Menu::calculateHeight()
{
    Rectangle<float> bounds;
    fontSize(font_size);
    fontFaceId(font);
    textBounds(0, 0, "item 1", nullptr, bounds);

    setHeight(display_number * (bounds.getHeight() + 4));
}

void Menu::setCallback(Callback *cb)
{
    callback = cb;
}
