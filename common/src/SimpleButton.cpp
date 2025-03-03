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
      fToggleValue(false),
      fDragAction(DragAction::NONE)
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

    Color textColor = text_color;
    Color backgroundColor = background_color;
    Color accentColor = accent_color;
    if (!fEnabled)
    {
        Color darker = Color(0, 0, 0);
        textColor.interpolate(darker, 0.5f);
        backgroundColor.interpolate(darker, 0.5f);
        accentColor.interpolate(darker, 0.5f);
    }

    // Background
    if (drawBackground || isToggle)
    {
        beginPath();
        if (fToggleValue)
            fillColor(accentColor);
        else
            fillColor(fHasFocus && fEnabled ? highlight_color : backgroundColor);
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
        fillColor(textColor);
    textAlign(ALIGN_CENTER | ALIGN_MIDDLE);
    text(width / 2, height / 2, label.c_str(), nullptr);
    closePath();
}

bool Button::onMouse(const MouseEvent &ev)
{
    if (
        fEnabled &&
        ev.press &&
        ev.button == kMouseButtonLeft &&
        contains(ev.pos))
    {
        fDragStartPos = ev.pos;
        fDragAction = DragAction::CLICKING;
        return true;
    }
    else if (
        fEnabled &&
        !ev.press &&
        fDragAction == DragAction::CLICKING &&
        contains(ev.pos))
    {
        if (isToggle)
            fToggleValue = !fToggleValue;

        if (callback != nullptr)
            callback->buttonClicked(this);

        fDragAction = DragAction::NONE;
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

        if (fDragAction == DragAction::CLICKING)
        {
            fDragAction = DragAction::DRAGGING;
            if (callback != nullptr)
                callback->buttonDragged(this);
        }
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