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
      border_color(Color(120, 120, 120))
{
}

void Menu::setFont(const char *name, const uchar *data, uint size)
{
    font = createFontFromMemory(name, data, size, false);
    repaint();
}

void Menu::addItem(const char *item)
{
    items.push_back(item);
}

void Menu::setDisplayNumber(int number)
{
    display_number = number;
    Rectangle<float> bounds;
    fontSize(font_size);
    fontFaceId(font);
    textBounds(0, 0, "item 1", nullptr, bounds);

    setHeight(display_number * (bounds.getHeight() + 4));
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

    for (int i = 0; i < display_number; i++)
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
    if (!isVisible())
        return false;

    if (ev.press && ev.button == MouseButton::kMouseButtonLeft)
    {
        if (!contains(ev.pos))
        {
            hide();
            return false;
        }

        if (highlighted_item < 0)
            return true;

        if (callback != nullptr)
            callback->onMenuItemSelection(this, highlighted_item);

        hide();
        repaint();
        return true;
    }

    return false;
}

bool Menu::onMotion(const MotionEvent &ev)
{
    if (!isVisible())
        return false;

    if (!contains(ev.pos))
    {
        hide(); // maybe not?
        return false;
    }

    const float height = getHeight();
    const float item_height = height / display_number;
    highlighted_item = scroll_index + std::floor(ev.pos.getY() / item_height);

    repaint();
    return true;
}

bool Menu::onScroll(const ScrollEvent &ev)
{
    if (!isVisible())
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

void Menu::setCallback(Callback *cb)
{
    callback = cb;
}