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
      align(Align::ALIGN_LEFT),
      textType(TextType::STRING),
      callback(nullptr)
{
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

    if (renderDebug)
    {
        beginPath();
        strokeColor(accent_color);
        rect(0, 0, width, height);
        stroke();
        closePath();
    }

    beginPath();
    if (hasKeyFocus)
        strokeColor(accent_color);
    else
        strokeColor(foreground_color);
    strokeWidth(1.f);
    rect(1, 1, width - 2, height - 2);
    stroke();
    closePath();

    if (textValue.size() == 0 && placeholder.length() > 0)
    {
        beginPath();
        fillColor(foreground_color);
        textAlign(ALIGN_MIDDLE | align);
        fontFaceId(font);
        fontSize(getFontSize());
        if (align == Align::ALIGN_CENTER)
            text(width / 2, height / 2, placeholder.c_str(), nullptr);
        else if (align == Align::ALIGN_LEFT)
            text(8, height / 2, placeholder.c_str(), nullptr);
        else
            text(width - 2, height / 2, placeholder.c_str(), nullptr);

        closePath();
    }

    Rectangle<float> bounds;

    if (textValue.size() > 0)
    {
        beginPath();
        fillColor(text_color);
        textAlign(ALIGN_MIDDLE | align);
        fontFaceId(font);
        fontSize(getFontSize());
        if (align == Align::ALIGN_CENTER)
            textBounds(width / 2, height / 2, textValue.c_str(), nullptr, bounds);
        else if (align == Align::ALIGN_LEFT)
            textBounds(2, height / 2, textValue.c_str(), nullptr, bounds);
        else
            textBounds(width - 2, height / 2, textValue.c_str(), nullptr, bounds);

        textAlign(ALIGN_LEFT | ALIGN_TOP);
        text(bounds.getX(), bounds.getY(), textValue.c_str(), nullptr);
        closePath();
    }
    else
    {
        if (align == Align::ALIGN_CENTER)
            bounds.setX(width / 2);
        else if (align == Align::ALIGN_LEFT)
            bounds.setX(2);
        else
            bounds.setX(width - 2);
    }

    if (renderDebug)
    {
        beginPath();
        strokeColor(Color(255, 0, 255));
        rect(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
        stroke();
        closePath();
    }

    if (!hasKeyFocus)
        return;

    // draw cursor
    beginPath();
    textAlign(ALIGN_MIDDLE | ALIGN_LEFT);
    fontFaceId(font);
    float x;
    if (textValue.size() > 0 && position > 0)
    {
        textBounds(bounds.getX(), bounds.getY(), textValue.substr(0, position).c_str(), nullptr, bounds);
        x = bounds.getX() + bounds.getWidth();
    }
    else
        x = bounds.getX();
    strokeColor(text_color);
    moveTo(x, 0);
    lineTo(x, height);
    stroke();
    closePath();
}

bool TextInput::onCharacterInput(const CharacterInputEvent &ev)
{
    if (!hasKeyFocus || !isVisible())
        return false;

    std::cout << "TextInput::onCharacterInput: ev.keycode = " << ev.keycode << std::endl;
    std::string candidate;
    candidate.assign(textValue);
    int newPosition = position;

    switch (ev.keycode)
    {
    // case 36:
    //     break;
    case kKeySpace:
        // space
        candidate.insert(candidate.begin() + newPosition, ev.string[0]);
        newPosition += 1;
        break;
    case kKeyBackspace:
    case kKeyTab:
    case kKeyDelete:
        break;
    default:
        // other characters
        candidate.insert(candidate.begin() + newPosition, ev.string[0]);
        newPosition += 1;
        break;
    }

    // Validate text here...
    bool valid = true;

    switch (textType)
    {
    case TextType::INTEGER:
        valid = isInteger(candidate.c_str());
        break;
    case TextType::FLOAT:
        valid = isFloat(candidate.c_str());
        break;
    case TextType::STRING:
        valid = true;
        break;
    default:
        break;
    }

    if (!valid)
        return true;

    textValue.assign(candidate);
    position = newPosition;

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
    case kKeyBackspace:
        if (position > 0)
        {
            textValue.erase(textValue.begin() + position - 1, textValue.begin() + position);
            position = position - 1;

            if (callback != nullptr)
                callback->textInputChanged(this, textValue);
        }
        break;
    case kKeyDelete:
        if (position < textValue.size())
        {
            textValue.erase(textValue.begin() + position, textValue.begin() + position + 1);
            if (callback != nullptr)
                callback->textInputChanged(this, textValue);
        }
        break;
    case kKeyEnter:
        if (callback != nullptr && textValue.compare(lastTextValue) != 0)
            callback->textEntered(this, textValue);

        hasKeyFocus = false;
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
    if (ev.press)
    {
        bool inWidget = contains(ev.pos);
        if (inWidget && !hasKeyFocus)
        {
            hasKeyFocus = true;
            lastTextValue.assign(textValue);
            position = textValue.size();
            repaint();
        }
        else if (!inWidget && hasKeyFocus)
        {
            if (callback != nullptr && textValue.size() > 0 && textValue.compare(lastTextValue) != 0)
                callback->textEntered(this, textValue);
            else if (textValue.size() == 0)
                textValue.assign(lastTextValue);
            hasKeyFocus = false;
            std::cout << "TextInput lost focus" << std::endl;
            repaint();
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
    }
    else if (hover && !over)
    {
        getWindow().setCursor(kMouseCursorArrow);
        hover = false;
    }

    return false;
}

bool TextInput::isInteger(const char *candidate)
{
    errno = 0;
    char *endptr;
    std::strtol(candidate, &endptr, 10);

    if (endptr == candidate)
        return false;
    else if (*endptr != '\0')
        return false;
    else
        return errno == 0;
}

bool TextInput::isFloat(const char *candidate)
{
    errno = 0;
    std::strtof(candidate, NULL);

    return errno == 0;
}

void TextInput::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO