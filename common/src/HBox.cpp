#include "HBox.hpp"

START_NAMESPACE_DISTRHO

HBox::HBox(Widget *widget) noexcept
    : WidgetGroup(widget),
      align_items(Align_Items::middle),
      justify_content(Justify_Content::space_between),
      padding(0)
{
    // setWidth(widget->getWidth());
    setHeight(50);
    setWidth(50);
}

void HBox::addWidget(NanoSubWidget *widget)
{
    addChildWidget(widget);
    items_.emplace_back(Item(widget));

    const float box_height = getHeight();
    const float wh = widget->getHeight();
    if (wh > box_height)
        setHeight(wh);

    for (auto it = items_.begin(); it != items_.end(); it++)
        it->height = getHeight();
}

void HBox::setWidgetAlignment(uint id, Align_Items a_i)
{
    for (auto it = items_.begin(); it != items_.end(); it++)
    {
        if (it->widget->getId() == id)
        {
            it->align_self = a_i;
            positionWidgets();
            return;
        }
    }
}
void HBox::setWidgetJustify_Content(uint id, Justify_Content j_c)
{
    for (auto it = items_.begin(); it != items_.end(); it++)
    {
        if (it->widget->getId() == id)
        {
            it->justify_content = j_c;
            positionWidgets();
            return;
        }
    }
}

void HBox::removeWidget(uint id)
{
    for (auto it = items_.begin(); it != items_.end(); it++)
    {
        if (it->widget->getId() == id)
        {
            items_.erase(it);
            positionWidgets();
            return;
        }
    }
}

void HBox::positionWidgets()
{
    // std::cout << "HBox::positionWidgets()" << std::endl;
    const float width = getWidth();
    const float height = getHeight();
    const float box_x = getAbsoluteX();
    const float box_y = getAbsoluteY();

    // printf("HBox::positionWidgets()\n  width = %.2f height = %.2f  box_x = %.2f box_y = %.2f\n", width, height, box_x, box_y);

    if (items_.size() == 0)
        return;

    switch (justify_content)
    {
    case Justify_Content::left:
    {
        float step = 0;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->x = box_x + step;
            it->widget->setAbsoluteX(it->x);
            it->widget->setAbsoluteY(box_y);
            const float ww = it->widget->getWidth();
            step += ww;
            step += padding;
            it->width = ww;
        }
        break;
    }
    case Justify_Content::right:
    {
        float combined_widget_width = 0;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            combined_widget_width += it->widget->getWidth();
            combined_widget_width += padding;
        }
        combined_widget_width -= padding;
        float startX = box_x + width - combined_widget_width;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->widget->setAbsoluteX(startX);
            it->widget->setAbsoluteY(box_y);
            it->x = startX;
            const float ww = it->widget->getWidth();
            startX += ww;
            startX += padding;
            it->width = ww;
        }

        break;
    }
    case Justify_Content::center:
    {
        float combined_widget_width = 0;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            combined_widget_width += it->widget->getWidth();
            combined_widget_width += padding;
        }

        combined_widget_width -= padding;

        int startX = box_x + width / 2 - combined_widget_width / 2;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->widget->setAbsoluteX(startX);
            it->widget->setAbsoluteY(box_y);
            it->x = startX;
            const float ww = it->widget->getWidth();
            startX += ww;
            startX += padding;
            it->width = ww;
        }
        break;
    }
    case Justify_Content::space_evenly:
    {
        float number_of_items = items_.size();
        float item_width = width / number_of_items;
        float step = 0;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            switch (it->justify_content)
            {
            case Justify_Content::left:
                it->widget->setAbsoluteX(box_x + step);
                break;
            case Justify_Content::right:
                it->widget->setAbsoluteX(box_x + step + item_width - it->widget->getWidth());
                break;
            case Justify_Content::center:
            case Justify_Content::space_evenly:
            case Justify_Content::none:
            default:
                const float ww = it->widget->getWidth();
                it->widget->setAbsoluteX(box_x + step + (item_width / 2 - ww / 2));
                break;
            }
            it->x = box_x + step;
            it->widget->setAbsoluteY(box_y);
            it->width = item_width;
            step += item_width;
        }
    }
    case Justify_Content::space_between:
    {
        float number_of_items = items_.size();
        float combined_widget_width = 0;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            combined_widget_width += it->widget->getWidth();
        };

        int space_left = width - combined_widget_width;
        int space_between = 0;
        if (number_of_items <= 1)
        {
            space_between = space_left;
        }
        else
        {
            space_between = space_left / (number_of_items - 1);
        }
        if (space_between < 0)
            space_between = 0;

        int startX = box_x;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->widget->setAbsoluteX(startX);
            it->widget->setAbsoluteY(box_y);
            it->x = startX;
            const float ww = it->widget->getWidth();
            startX += ww + space_between;
            it->width = ww;
        }
        break;
    }
    default:
        break;
    }

    switch (align_items)
    {
    case Align_Items::top:
    {
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->widget->setAbsoluteY(box_y);
        }
        break;
    }
    case Align_Items::middle:
    {
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->widget->setAbsoluteY(box_y + height / 2 - it->widget->getHeight() / 2);
        }
        break;
    }
    case Align_Items::bottom:
    {
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->widget->setAbsoluteY(box_y + height - it->widget->getHeight());
        }
        break;
    }
    default:
        break;
    }

    for (auto it = items_.begin(); it != items_.end(); it++)
    {
        switch (it->align_self)
        {
        case Align_Items::top:
            it->widget->setAbsoluteY(box_y);
            break;
        case Align_Items::middle:
            it->widget->setAbsoluteY(box_y + height / 2 - it->widget->getHeight() / 2);
            break;
        case Align_Items::bottom:
            it->widget->setAbsoluteY(box_y + height - it->height);
            break;
        case Align_Items::none:
        default:
            break;
        }
    }

    for (auto it = items_.begin(); it != items_.end(); it++)
    {
        float item_x = it->x;
        float item_w = it->width;
        switch (it->justify_content)
        {
        case Justify_Content::left:
            it->widget->setAbsoluteX(item_x);
            break;
        case Justify_Content::right:
            it->widget->setAbsoluteX(item_x + item_w - it->widget->getWidth());
            /* code */
            break;
        // case Justify_Content::center:
        // case Justify_Content::space_evenly:
        // case Justify_Content::none:
        default:

            break;
        }
    }
}

void HBox::resizeToFit()
{
    float width = 0.0f;
    float height = 0.0f;

    for (auto it = items_.begin(); it != items_.end(); it++)
    {
        width += it->widget->getWidth();
        height = std::max(height, (float)it->widget->getHeight());
    }

    if (items_.size() > 0)
        width += (items_.size() - 1) * padding;

    setWidth(width);
    setHeight(height);
}

END_NAMESPACE_DISTRHO