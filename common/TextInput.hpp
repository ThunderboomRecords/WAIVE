#ifndef TEXT_INPUT_HPP_INCLUDED
#define TEXT_INPUT_HPP_INCLUDED

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include <iostream>

START_NAMESPACE_DISTRHO

class TextInput : public NanoSubWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void textEntered(TextInput *textInput, std::string text) = 0;
    };

    explicit TextInput(Widget *widget) noexcept;

    void setCallback(Callback *cb);
    void setText(const char *text, bool sendCallback = false);

    float font_size;
    Color background_color, text_color;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onCharacterInput(const CharacterInputEvent &) override;
    bool onKeyboard(const KeyboardEvent &) override;

private:
    Callback *callback;

    std::string fText, fTextStart;
    bool hasKeyFocus;
    int position;
};

END_NAMESPACE_DISTRHO

#endif