#include "RadioButtons.hpp"

RadioButtons::RadioButtons(Widget *parent) noexcept
    : Menu(parent),
      radioSize(4.f)
{
}

bool RadioButtons::onMouse(const MouseEvent &ev)
{
    if (!isVisible() || items.size() == 0 || !contains(ev.pos))
        return false;

    if (ev.press && ev.button == MouseButton::kMouseButtonLeft)
    {
        const float item_height = getHeight() / (float)items.size();
        highlighted_item = std::floor(ev.pos.getY() / item_height);

        if (callback != nullptr)
            callback->onMenuItemSelection(this, highlighted_item, items[highlighted_item]);

        getParentWidget()->repaint();
        return true;
    }

    return false;
}

bool RadioButtons::onMotion(const MotionEvent &ev)
{
    return false;
}

bool RadioButtons::onKeyboard(const KeyboardEvent &ev)
{
    return false;
}

void RadioButtons::onNanoDisplay()
{
    if (!isVisible() || items.size() == 0)
        return;

    const float width = getWidth();
    const float height = getHeight();
    const float item_height = height / (float)items.size();

    // beginPath();
    // fillColor(background_color);
    // rect(0, 0, width, height);
    // fill();
    // closePath();

    const float half_item_height = item_height / 2.f;

    for (size_t i = 0; i < items.size(); i++)
    {
        float y = i * item_height;

        beginPath();
        fontSize(getFontSize());
        fontFaceId(font);
        strokeColor(text_color);
        strokeWidth(1.f);
        fillColor(text_color);
        if (alignment == Align::ALIGN_LEFT)
        {
            circle(radioSize + 1.f, y + half_item_height, radioSize);
            stroke();

            if (i == highlighted_item)
            {
                circle(radioSize + 1, y + half_item_height, radioSize - 4.f);
                fill();
            }

            textAlign(Align::ALIGN_LEFT | Align::ALIGN_MIDDLE);
            text(radioSize * 2.f + 10.f, y + half_item_height, items[scroll_index + i].c_str(), NULL);
        }
        else
        {
            circle(width - radioSize - 1.f, y + half_item_height, radioSize);
            stroke();

            if (i == highlighted_item)
            {
                circle(width - radioSize - 1.f, y + half_item_height, radioSize - 4.f);
                fill();
            }

            textAlign(Align::ALIGN_RIGHT | Align::ALIGN_MIDDLE);
            text(radioSize * 2.f - 10.f, y + half_item_height, items[scroll_index + i].c_str(), NULL);
        }
        closePath();
    }
}

void RadioButtons::calculateWidth()
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

    setWidth(maxWidth + radioSize * 2.f + 10.f);
}