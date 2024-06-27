#ifndef TEXT_INPUT_HPP_INCLUDED
#define TEXT_INPUT_HPP_INCLUDED

#include "WAIVEWidget.hpp"
#include <iostream>

START_NAMESPACE_DISTRHO

class TextInput : public WAIVEWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void textEntered(TextInput *textInput, std::string text) = 0;
        virtual void textInputChanged(TextInput *textInput, std::string text) = 0;
    };

    explicit TextInput(Widget *widget) noexcept;

    void setCallback(Callback *cb);
    void setText(const char *text, bool sendCallback = false);
    void undo();

    std::string placeholder;
    Align align;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onCharacterInput(const CharacterInputEvent &) override;
    bool onKeyboard(const KeyboardEvent &) override;

private:
    Callback *callback;

    std::string textValue, lastTextValue;
    bool hasKeyFocus, hover;
    int position;
};

END_NAMESPACE_DISTRHO

#endif