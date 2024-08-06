#include "VBox.hpp"

START_NAMESPACE_DISTRHO

VBox::VBox(Widget *widget) noexcept
    : WidgetGroup(widget),
      align_items(Align_Items::middle),
      justify_content(Justify_Content::space_between),
      padding(0)
{
    setHeight(50);
    setWidth(50);
}

void VBox::addWidget(NanoSubWidget *widget)
{
    addChildWidget(widget);
    items_.emplace_back(Item(widget));

    const float box_width = getWidth();
    const float ww = widget->getWidth();
    if (ww > box_width)
        setWidth(ww);

    for (auto it = items_.begin(); it != items_.end(); it++)
        it->width = box_width;
}

void VBox::setWidgetAlignment(uint id, Align_Items a_i)
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
void VBox::setWidgetJustify_Content(uint id, Justify_Content j_c)
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

void VBox::removeWidget(uint id)
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

void VBox::positionWidgets()
{
    const float width = getWidth();
    const float height = getHeight();
    const float box_x = getAbsoluteX();
    const float box_y = getAbsoluteY();

    // printf("VBox::positionWidgets()\n  width = %.2f height = %.2f  box_x = %.2f box_y = %.2f\n", width, height, box_x, box_y);

    if (items_.size() == 0)
        return;

    switch (justify_content)
    {
    case Justify_Content::top:
    {
        float step = 0;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->y = box_y + step;
            it->widget->setAbsoluteX(box_x);
            it->widget->setAbsoluteY(it->y);
            const float wh = it->widget->getHeight();
            step += wh;
            step += padding;
            it->height = wh;
        }
        break;
    }
    case Justify_Content::bottom:
    {
        float combined_widget_height = 0;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            combined_widget_height += it->widget->getHeight();
            combined_widget_height += padding;
        }
        combined_widget_height -= padding;
        float startY = box_y + height - combined_widget_height;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->widget->setAbsoluteY(startY);
            it->widget->setAbsoluteX(box_x);
            it->y = startY;
            const float wh = it->widget->getHeight();
            startY += wh;
            startY += padding;
            it->height = wh;
        }

        break;
    }
    case Justify_Content::center:
    {
        float combined_widget_height = 0;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            combined_widget_height += it->widget->getHeight();
            combined_widget_height += padding;
        }
        combined_widget_height -= padding;

        int startY = box_y + height / 2 - combined_widget_height / 2;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->widget->setAbsoluteY(startY);
            it->widget->setAbsoluteX(box_x);
            it->y = startY;
            const float wh = it->widget->getHeight();
            startY += wh;
            startY += padding;
            it->height = wh;
        }
        break;
    }
    case Justify_Content::space_evenly:
    {
        float number_of_items = items_.size();
        float item_height = height / number_of_items;
        float step = 0;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            switch (it->justify_content)
            {
            case Justify_Content::top:
                it->widget->setAbsoluteY(box_y + step);
                break;
            case Justify_Content::bottom:
                it->widget->setAbsoluteY(box_y + step + item_height - it->widget->getHeight());
                break;
            case Justify_Content::center:
            case Justify_Content::space_evenly:
            case Justify_Content::none:
            default:
                const float wh = it->widget->getHeight();
                it->widget->setAbsoluteY(box_y + step + (item_height / 2 - wh / 2));
                break;
            }
            it->y = box_y + step;
            it->widget->setAbsoluteX(box_x);
            it->height = item_height;
            step += item_height;
        }
    }
    case Justify_Content::space_between:
    {
        float number_of_items = items_.size();
        float combined_widget_height = 0;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            combined_widget_height += it->widget->getHeight();
        };

        int space_left = height - combined_widget_height;
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

        int startY = box_y;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->widget->setAbsoluteY(startY);
            it->widget->setAbsoluteX(box_x);
            it->y = startY;
            const float wh = it->widget->getHeight();
            startY += wh + space_between;
            it->height = wh;
        }
        break;
    }
    default:
        break;
    }

    switch (align_items)
    {
    case Align_Items::left:
    {
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->widget->setAbsoluteX(box_x);
        }
        break;
    }
    case Align_Items::middle:
    {
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->widget->setAbsoluteX(box_x + width / 2 - it->widget->getWidth() / 2);
        }
        break;
    }
    case Align_Items::right:
    {
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->widget->setAbsoluteX(box_x + width - it->widget->getWidth());
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
        case Align_Items::left:
            it->widget->setAbsoluteX(box_x);
            break;
        case Align_Items::middle:
            it->widget->setAbsoluteX(box_x + width / 2 - it->widget->getWidth() / 2);
            break;
        case Align_Items::right:
            it->widget->setAbsoluteX(box_x + width - it->width);
            break;
        case Align_Items::none:
        default:
            break;
        }
    }

    for (auto it = items_.begin(); it != items_.end(); it++)
    {
        float item_y = it->y;
        float item_h = it->height;
        switch (it->justify_content)
        {
        case Justify_Content::top:
            it->widget->setAbsoluteY(item_y);
            break;
        case Justify_Content::bottom:
            it->widget->setAbsoluteY(item_y + item_h - it->widget->getHeight());
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

void VBox::resizeToFit()
{
    float height = 0.0f;
    float width = 0.0f;

    for (auto it = items_.begin(); it != items_.end(); it++)
    {
        height += it->widget->getHeight();
        width = std::max(width, (float)it->widget->getWidth());
    }

    if (items_.size() > 0)
        height += (items_.size() - 1) * padding;

    setHeight(height);
    setWidth(width);
}

END_NAMESPACE_DISTRHO