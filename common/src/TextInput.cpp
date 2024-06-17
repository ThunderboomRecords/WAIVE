#include "TextInput.hpp"

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::Color;

TextInput::TextInput(Widget *parent) noexcept
    : WAIVEWidget(parent),
      hasKeyFocus(false),
      hover(false),
      textValue(""),
      position(0),
      placeholder(""),
      callback(nullptr)
{
    loadSharedResources();
}

void TextInput::setText(const char *newText, bool sendCallback)
{
    textValue.assign(newText);
    position = textValue.size();

    if (sendCallback && callback != nullptr)
        callback->textEntered(this, textValue);

    repaint();
}

void TextInput::undo()
{
    // In case the text is invalid by reciever of callback,
    // can revert the text back to it's original
    textValue.assign(lastTextValue);
    if (callback != nullptr)
        callback->textInputChanged(this, textValue);
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

    if (hasKeyFocus)
    {
        beginPath();
        strokeColor(accent_color);
        strokeWidth(1);
        rect(1, 1, width - 2, height - 2);
        stroke();
        closePath();
    }
    else if (textValue.size() == 0 && placeholder.length() > 0)
    {
        beginPath();
        fillColor(foreground_color);
        textAlign(ALIGN_TOP);
        fontFaceId(0);
        text(4, 2, placeholder.c_str(), nullptr);
        closePath();
    }

    if (textValue.size() > 0)
    {
        beginPath();
        fillColor(text_color);
        textAlign(ALIGN_TOP);
        fontFaceId(0);
        text(4, 2, textValue.c_str(), nullptr);
        closePath();
    }

    if (!hasKeyFocus)
        return;

    // draw cursor
    if (textValue.size() > 0 && position > 0)
    {
        Rectangle<float> bounds;
        beginPath();
        textAlign(ALIGN_TOP);
        fontFaceId(0);
        textBounds(4, 2, textValue.substr(0, position).c_str(), nullptr, bounds);
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

    switch (ev.keycode)
    {
    case 36:
        // enter
        if (callback != nullptr && textValue.compare(lastTextValue) != 0)
            callback->textEntered(this, textValue);

        hasKeyFocus = false;
        break;
    case 22:
        // backspace
        if (position > 0)
        {
            textValue.erase(textValue.begin() + position - 1, textValue.begin() + position);
            position = position - 1;
        }
        break;
    case 65:
        // space
        textValue.insert(textValue.begin() + position, ev.string[0]);
        position += 1;
        break;
    case 119:
        // delete key
        if (position < textValue.size())
            textValue.erase(textValue.begin() + position, textValue.begin() + position + 1);
        break;
    case 23:
        // tab key
        break;
    default:
        // other characters
        textValue.insert(textValue.begin() + position, ev.string[0]);
        position += 1;
        break;
    }

    if (callback != nullptr)
        callback->textInputChanged(this, textValue);

    repaint();
    return true;
}

bool TextInput::onKeyboard(const KeyboardEvent &ev)
{
    if (!hasKeyFocus || !ev.press)
        return false;

    switch (ev.key)
    {
    case kKeyLeft:
        if (position > 0)
            position -= 1;
        break;
    case kKeyRight:
        if (position < textValue.size())
            position += 1;
        break;
    case kKeyHome:
        position = 0;
        break;
    case kKeyEnd:
        position = textValue.size();
        break;
    case kKeyEscape:
        hasKeyFocus = false;
        textValue.assign(lastTextValue);
        if (callback != nullptr)
            callback->textInputChanged(this, textValue);
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
            lastTextValue.assign(textValue);
            position = textValue.size();
            repaint();
            return true;
        }
        else if (!contains(ev.pos) && hasKeyFocus)
        {
            if (callback != nullptr && textValue.size() > 0 && textValue.compare(lastTextValue) != 0)
                callback->textEntered(this, textValue);
            else if (textValue.size() == 0)
                textValue.assign(lastTextValue);
            hasKeyFocus = false;
            repaint();
            return true;
        }
    }

    return false;
}

bool TextInput::onMotion(const MotionEvent &ev)
{
    if (!isVisible())
        return false;

    bool over = contains(ev.pos);
    if (!hover && over)
    {
        getWindow().setCursor(kMouseCursorCaret);
        hover = true;
        return true;
    }
    else if (hover && !over)
    {
        getWindow().setCursor(kMouseCursorArrow);
        hover = false;
        return false;
    }

    return false;
}

void TextInput::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO