#include "VBox.hpp"

START_NAMESPACE_DISTRHO


VBox::VBox(Widget *widget) noexcept
    : NanoSubWidget(widget),
      align_items(Align_Items::middle),
      justify_content(Justify_Content::space_evenly),
      padding(0)
{
    setHeight(widget->getHeight());
}

void VBox::addWidget(SubWidget *widget)
{
    items_.emplace_back(Item(widget));

    const uint box_width = getWidth();
    const uint ww = widget->getWidth();
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
    const uint width = getWidth();
    const uint height = getHeight();
    const uint box_x = getAbsoluteX();
    const uint box_y = getAbsoluteY();

    printf("%d %d %d %d\n", width, height, box_x, box_y);

    switch (justify_content)
    {
    case Justify_Content::top:
    {
        uint step = 0;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->y = box_y + step;
            it->widget->setAbsoluteX(box_x);
            it->widget->setAbsoluteY(it->y);
            const uint wh = it->widget->getHeight();
            step += wh;
            step += padding;
            it->height = wh;
        }
        break;
    }
    case Justify_Content::bottom:
    {
        uint combined_widget_height = 0;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            combined_widget_height += it->widget->getHeight();
            combined_widget_height += padding;
        }
        combined_widget_height -= padding;
        uint startY = box_y + height - combined_widget_height;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->widget->setAbsoluteY(startY);
            it->widget->setAbsoluteX(box_x);
            it->y = startY;
            const uint wh = it->widget->getHeight();
            startY += wh;
            startY += padding;
            it->width = wh;
        }

        break;
    }
    case Justify_Content::center:
    {
        uint combined_widget_height = 0;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            combined_widget_height += it->widget->getHeight();
        }

        int startY = box_y + height / 2 - combined_widget_height / 2;
        for (auto it = items_.begin(); it != items_.end(); it++)
        {
            it->widget->setAbsoluteY(startY);
            it->widget->setAbsoluteX(box_x);
            it->y = startY;
            const uint wh = it->widget->getHeight();
            startY += wh;
            it->height = wh;
        }
        break;
    }
    case Justify_Content::space_evenly:
    {
        uint number_of_items = items_.size();
        uint item_height = height / number_of_items;
        uint step = 0;
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
                const uint wh = it->widget->getHeight();
                it->widget->setAbsoluteY(box_y + step + (item_height / 2 - wh / 2));
                break;
            }
            it->y = box_y + step;
            it->widget->setAbsoluteX(box_x);
            it->height = item_height;
            step += item_height;
        }
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
        uint item_y = it->y;
        uint item_h = it->height;
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

void VBox::onNanoDisplay()
{
    beginPath();
    strokeColor(255, 0, 0);
    rect(0, 0, getWidth(), getHeight());
    stroke();
    closePath();
}

END_NAMESPACE_DISTRHO