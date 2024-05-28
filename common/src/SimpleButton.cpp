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
      callback(nullptr),
      fEnabled(true)
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

    // Background
    beginPath();
    if (fHasFocus)
        fillColor(Color(backgroundColor, Color(255, 255, 255), 0.5f));
    else
        fillColor(backgroundColor);
    strokeColor(labelColor);
    rect(margin, margin, width - 2 * margin, height - 2 * margin);
    fill();
    stroke();
    closePath();

    // Label
    beginPath();
    fontSize(14 * fontScale);
    fillColor(labelColor);
    Rectangle<float> bounds;
    textBounds(0, 0, label.c_str(), NULL, bounds);
    float tx = width / 2.0f;
    float ty = height / 2.0f;
    textAlign(ALIGN_CENTER | ALIGN_MIDDLE);
    text(tx, ty, label.c_str(), NULL);
    closePath();

    if (!fEnabled)
    {
        beginPath();
        fillColor(0.f, 0.f, 0.f, 0.3f);
        rect(0, 0, width, height);
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
        if (!fHasFocus)
        {
            fHasFocus = true;
            getWindow().setCursor(kMouseCursorHand);
            repaint();
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

void Button::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DGL