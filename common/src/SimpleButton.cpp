#include "SimpleButton.hpp"

START_NAMESPACE_DGL

Button::Button(Widget *parent)
    : WAIVEWidget(parent),
      label("button"),
      fHasFocus(false),
      callback(nullptr),
      fEnabled(true),
      drawBackground(true),
      isToggle(false),
      fToggleValue(false)
{
    background_color = WaiveColors::grey2;
    accent_color = WaiveColors::light2;
}

void Button::setLabel(const std::string &label_)
{
    label = label_;
    repaint();
}

void Button::resizeToFit()
{
    if (label.length() == 0)
        return;

    fontSize(getFontSize());
    fontFaceId(font);

    Rectangle<float> bounds;
    textBounds(0, 0, label.c_str(), NULL, bounds);

    setHeight(getFontSize() * 2.f);
    setWidth(bounds.getWidth() + getHeight());
}

void Button::setEnabled(bool enabled)
{
    fEnabled = enabled;
    repaint();
}

void Button::onNanoDisplay()
{
    const uint width = getWidth();
    const uint height = getHeight();

    if (renderDebug)
    {
        beginPath();
        strokeColor(accent_color);
        rect(0, 0, width, height);
        stroke();
        closePath();
    }

    // Background
    if (drawBackground || isToggle)
    {
        beginPath();
        if (fToggleValue)
            fillColor(accent_color);
        else
            fillColor(fHasFocus ? highlight_color : background_color);
        roundedRect(0, 0, width, height, height * 0.5f);
        fill();
        closePath();
    }

    // Label
    beginPath();
    fontSize(getFontSize());
    fontFaceId(font);
    if (isToggle && fToggleValue)
        fillColor(WaiveColors::dark);
    else
        fillColor(text_color);
    textAlign(ALIGN_CENTER | ALIGN_MIDDLE);
    text(width / 2, height / 2, label.c_str(), nullptr);
    closePath();

    if (!fEnabled && drawBackground)
    {
        beginPath();
        fillColor(0.f, 0.f, 0.f, 0.5f);
        roundedRect(0, 0, width, height, height * 0.5f);
        fill();
        closePath();
    }
}

bool Button::onMouse(const MouseEvent &ev)
{
    // std::cout << "Button::onMouse " << description << std::endl;
    if (
        fEnabled &&
        ev.press &&
        ev.button == kMouseButtonLeft &&
        contains(ev.pos))
    {
        if (isToggle)
            fToggleValue = !fToggleValue;

        if (callback != nullptr)
            callback->buttonClicked(this);

        repaint();
        return true;
    }

    return false;
}

bool Button::onMotion(const MotionEvent &ev)
{
    if (contains(ev.pos))
    {
        if (!fHasFocus && fEnabled)
        {
            fHasFocus = true;
            getWindow().setCursor(kMouseCursorHand);
            repaint();
        }
        return false;
    }
    else
    {
        if (fHasFocus)
        {
            fHasFocus = false;
            getWindow().setCursor(kMouseCursorArrow);
            repaint();
        }
    }
    return false;
}

void Button::setToggled(bool value, bool sendCallback)
{
    fToggleValue = value;
    repaint();
    if (callback != nullptr && sendCallback)
        callback->buttonClicked(this);
}

bool Button::getToggled() const
{
    return fToggleValue;
}

void Button::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DGL