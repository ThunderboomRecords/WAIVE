#include "Popup.hpp"

START_NAMESPACE_DISTRHO

Popup::Popup(Widget *widget, float x, float y, float width, float height, bool ignore_sf)
    : WidgetGroup(widget, x, y, width, height, ignore_sf),
      border_radius(7.0f),
      callback(nullptr)
{
    close_btn = new Button(widget);
    close_btn->setLabel("✕");
    close_btn->setSize(24, 24);
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
    strokeColor(accent_color);
    strokeWidth(2.f);
    roundedRect(1, 1, width - 1, height - 1, border_radius);
    fill();
    stroke();
    closePath();

    if (title.length() > 0)
    {
        beginPath();
        fillColor(text_color);
        fontSize(getFontSize());
        fontFaceId(font);
        textAlign(ALIGN_CENTER | ALIGN_TOP);
        text(width / 2.f, 4, title.c_str(), nullptr);
        closePath();
    }
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
    show();

    if (callback != nullptr)
        callback->popupOpened(this);
}

void Popup::close()
{
    setVisible(false);
    NanoSubWidget::setVisible(false);
    toBottom();
    hide();

    if (callback != nullptr)
        callback->popupClosed(this);
}

void Popup::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO