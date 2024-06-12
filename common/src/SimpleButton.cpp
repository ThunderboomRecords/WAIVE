#include "SimpleButton.hpp"

START_NAMESPACE_DGL

Button::Button(Widget *parent)
    : WAIVEWidget(parent),
      label("button"),
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

void Button::setLabel(const std::string &label_)
{
    label = label_;
    repaint();
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
        fillColor(Color(background_color.red + 0.1f, background_color.green + 0.1f, background_color.blue + 0.1f, background_color.alpha));
    else
        fillColor(background_color);
    strokeColor(text_color);
    rect(margin, margin, width - 2 * margin, height - 2 * margin);
    fill();
    stroke();
    closePath();

    // Label
    beginPath();
    fontSize(14);
    fillColor(text_color);
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
        std::cout << "Button::onMouse\n";
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