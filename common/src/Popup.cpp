#include "Popup.hpp"

START_NAMESPACE_DISTRHO

Popup::Popup(Widget *widget) : WidgetGroup(widget),
                               background_color(Color(200, 200, 200)),
                               border_color(Color(30, 30, 30)),
                               border_radius(4.0f)
{
    close_btn = new Button(widget);
    close_btn->setLabel("x");
    close_btn->setSize(20, 20);
    close_btn->setCallback(this);

    addChildWidget(close_btn);
}

void Popup::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(background_color);
    strokeColor(border_color);
    roundedRect(0, 0, width, height, border_radius);
    fill();
    stroke();
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
    return false;
}

void Popup::buttonClicked(Button *button)
{
    if (button == close_btn)
        close();
}

void Popup::open()
{
    std::cout << "Popup::open\n";
    toFront();
    Layout::onTop(close_btn, this, Widget_Align::END, Widget_Align::START, 10);
    NanoSubWidget::setVisible(true);
    setVisible(true);
}

void Popup::close()
{
    std::cout << "Popup::close\n";
    setVisible(false);
    NanoSubWidget::setVisible(false);
    toBottom();
    getParentWidget()->repaint();
}

END_NAMESPACE_DISTRHO