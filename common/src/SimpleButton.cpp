#include "SimpleButton.hpp"
#include "Window.hpp"

START_NAMESPACE_DGL

Button::Button(Widget *parent)
    : NanoSubWidget(parent),
      backgroundColor(32, 32, 32),
      labelColor(255, 255, 255),
      label("button"),
      fontScale(1.0f),
      fHasFocus(false),
      callback(nullptr)
{
#ifdef DGL_NO_SHARED_RESOURCES
    createFontFromFile("sans", "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf");
#else
    loadSharedResources();
#endif
}

Button::~Button()
{
}

void Button::setBackgroundColor(const Color color)
{
    backgroundColor = color;
}

void Button::setFontScale(const float scale)
{
    fontScale = scale;
}

void Button::setLabel(const std::string &label2)
{
    label = label2;
}

void Button::setLabelColor(const Color color)
{
    labelColor = color;
}

void Button::onNanoDisplay()
{
    const uint w = getWidth();
    const uint h = getHeight();
    const float margin = 1.0f;

    // Background
    beginPath();
    fillColor(backgroundColor);
    strokeColor(labelColor);
    rect(margin, margin, w - 2 * margin, h - 2 * margin);
    fill();
    stroke();
    closePath();

    // Label
    beginPath();
    fontSize(14 * fontScale);
    fillColor(labelColor);
    Rectangle<float> bounds;
    textBounds(0, 0, label.c_str(), NULL, bounds);
    float tx = w / 2.0f;
    float ty = h / 2.0f;
    textAlign(ALIGN_CENTER | ALIGN_MIDDLE);
    text(tx, ty, label.c_str(), NULL);
    closePath();
}

bool Button::onMouse(const MouseEvent &ev)
{
    if (callback != nullptr && ev.press && ev.button == 1 && contains(ev.pos))
    {
        callback->buttonClicked(this);
        return true;
    }

    return false;
}

bool Button::onMotion(const MotionEvent &ev)
{
    bool hover = contains(ev.pos);
    if (hover)
    {
        if (!fHasFocus)
        {
            fHasFocus = true;
            getWindow().setCursor(kMouseCursorHand);
        }
    }
    else
    {
        if (fHasFocus)
        {
            fHasFocus = false;
            getWindow().setCursor(kMouseCursorArrow);
        }
    }
    return false;
}

void Button::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DGL