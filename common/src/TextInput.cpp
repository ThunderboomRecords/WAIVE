#include "TextInput.hpp"

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::Color;

TextInput::TextInput(Widget *parent) noexcept
    : NanoSubWidget(parent),
      hasKeyFocus(false),
      fText(""),
      position(0),
      background_color(Color(230, 230, 230)),
      text_color(Color(30, 30, 30))
{
    loadSharedResources();
}

void TextInput::setText(const char *newText, bool sendCallback)
{
    fText.assign(newText);
    position = fText.size();

    if (sendCallback && callback != nullptr)
        callback->textEntered(this, fText);

    repaint();
}

void TextInput::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(background_color);
    rect(0, 0, width, height);
    fill();
    closePath();

    if (fText.size() > 0)
    {
        beginPath();
        fillColor(text_color);
        textAlign(ALIGN_TOP);
        fontFaceId(0);
        text(4, 2, fText.c_str(), nullptr);
        closePath();
    }

    if (!hasKeyFocus)
        return;

    // draw cursor
    if (fText.size() > 0 && position > 0)
    {
        Rectangle<float> bounds;
        beginPath();
        textAlign(ALIGN_TOP);
        fontFaceId(0);
        textBounds(4, 2, fText.substr(0, position).c_str(), nullptr, bounds);
        strokeColor(text_color);
        float x = bounds.getX() + bounds.getWidth();
        moveTo(x, 0);
        lineTo(x, height);
        stroke();
        closePath();
    }
    else
    {
        beginPath();
        strokeColor(text_color);
        moveTo(4, 0);
        lineTo(4, height);
        stroke();
        closePath();
    }
}

bool TextInput::onCharacterInput(const CharacterInputEvent &ev)
{
    if (!hasKeyFocus || !isVisible())
        return false;

    std::cout << "TextInput::onCharacterInput: " << ev.string << " " << ev.keycode << std::endl;
    switch (ev.keycode)
    {
    case 36:
        // enter
        if (callback != nullptr && fText.size() > 0 && fText.compare(fTextStart) != 0)
            callback->textEntered(this, fText);
        else if (fText.size() == 0)
            fText.assign(fTextStart);
        hasKeyFocus = false;
        break;
    case 22:
        if (position > 0)
        {
            fText.erase(fText.begin() + position - 1, fText.begin() + position);
            position = position - 1;
        }
        break;
    case 65:
        // space
        fText.insert(fText.begin() + position, ev.string[0]);
        position += 1;
        break;
    case 119:
        // delete key
        if (position < fText.size())
        {
            fText.erase(fText.begin() + position, fText.begin() + position + 1);
        }
        break;
    case 23:
        // tab key
        break;
    default:
        // other characters
        fText.insert(fText.begin() + position, ev.string[0]);
        position += 1;
        break;
    }

    std::cout << fText << " " << position << std::endl;
    repaint();
    return true;
}

bool TextInput::onKeyboard(const KeyboardEvent &ev)
{
    if (!hasKeyFocus)
        return false;

    switch (ev.key)
    {
    case kKeyLeft:
        if (position > 0)
            position -= 1;
        break;
    case kKeyRight:
        if (position < fText.size())
            position += 1;
        break;
    case kKeyHome:
        position = 0;
        break;
    case kKeyEnd:
        position = fText.size();
        break;
    case kKeyEscape:
        hasKeyFocus = false;
        fText.assign(fTextStart);
        break;
    default:
        return false;
        break;
    }

    repaint();
    return true;
}

bool TextInput::onMouse(const MouseEvent &ev)
{
    if (!isVisible())
        return false;

    if (ev.press)
    {
        if (contains(ev.pos) && !hasKeyFocus)
        {
            hasKeyFocus = true;
            fTextStart.assign(fText);
            position = fText.size();
            repaint();
            return true;
        }
        else if (!contains(ev.pos) && hasKeyFocus)
        {
            hasKeyFocus = false;
            repaint();
            return true;
        }
    }

    return false;
}

void TextInput::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO