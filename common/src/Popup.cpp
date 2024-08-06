#include "Popup.hpp"

START_NAMESPACE_DISTRHO

Popup::Popup(Widget *widget, float x, float y, float width, float height)
    : WidgetGroup(widget, x, y, width, height),
      border_radius(7.0f)
{
    close_btn = new Button(widget);
    close_btn->setLabel("x");
    close_btn->setSize(20, 20);
    close_btn->setCallback(this);
    close_btn->onTop(this, Widget_Align::END, Widget_Align::START, 10);

    addChildWidget(close_btn);
}

void Popup::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(background_color);
    roundedRect(0, 0, width, height, border_radius);
    fill();
    closePath();
}

bool Popup::onMouse(const MouseEvent &ev)
{
    if (!isVisible())
        return false;

    if (!contains(ev.pos))
    {
        close();
        return true;
    }
    return true;
}

void Popup::buttonClicked(Button *button)
{
    if (button == close_btn)
        close();
}

void Popup::open()
{
    NanoSubWidget::toFront();
    toFront();
    NanoSubWidget::setVisible(true);
    setVisible(true);
}

void Popup::close()
{
    setVisible(false);
    NanoSubWidget::setVisible(false);
    toBottom();
    getParentWidget()->repaint();
}

END_NAMESPACE_DISTRHO