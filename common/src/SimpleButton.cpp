#include "SimpleButton.hpp"

START_NAMESPACE_DGL

Button::Button(Widget *parent)
    : WAIVEWidget(parent),
      label("button"),
      fHasFocus(false),
      callback(nullptr),
      fEnabled(true),
      drawBackground(true)
{
    loadSharedResources();
    background_color = WaiveColors::grey2;
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

    fontSize(font_size);
    fontFaceId(font);

    Rectangle<float> bounds;
    textBounds(0, 0, label.c_str(), NULL, bounds);

    setHeight(bounds.getHeight() * 2.f);
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
    const float margin = 1.0f;

    if (renderDebug)
    {
        beginPath();
        strokeColor(accent_color);
        rect(0, 0, width, height);
        stroke();
        closePath();
    }

    // Background
    if (drawBackground)
    {
        beginPath();
        fillColor(fHasFocus ? highlight_color : background_color);
        roundedRect(margin, margin, width - 2 * margin, height - 2 * margin, height * 0.5f);
        fill();
        closePath();
    }

    // Label
    beginPath();
    fontSize(font_size);
    fontFaceId(font);
    fillColor(text_color);
    textAlign(ALIGN_CENTER | ALIGN_MIDDLE);
    text(width / 2, height / 2, label.c_str(), nullptr);
    closePath();

    if (!fEnabled)
    {
        beginPath();
        fillColor(0.f, 0.f, 0.f, 0.5f);
        roundedRect(margin, margin, width - 2 * margin, height - 2 * margin, height * 0.5f);
        fill();
        closePath();
    }
}

bool Button::onMouse(const MouseEvent &ev)
{
    if (
        fEnabled &&
        callback != nullptr &&
        ev.press &&
        ev.button == 1 &&
        contains(ev.pos))
    {
        callback->buttonClicked(this);
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
        return true;
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

void Button::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DGL