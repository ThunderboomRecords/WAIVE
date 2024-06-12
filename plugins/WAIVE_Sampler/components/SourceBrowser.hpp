#ifndef SOURCE_BROWSER_HPP_INCLUDED
#define SOURCE_BROWSER_HPP_INCLUDED

// #include "Popup.hpp"
#include "NanoVG.hpp"
#include "Window.hpp"
#include "CheckboxGroup.hpp"

START_NAMESPACE_DISTRHO

class SourceBrowser
    // : public NanoTopLevelWidget,
    : public NanoStandaloneWindow,
      CheckboxGroup::Callback
{
public:
    // explicit SourceBrowser(Widget *widget, float x, float y, float width, float height) noexcept;
    explicit SourceBrowser(Application &app, float width, float height);

protected:
    void checkboxesUpdated(CheckboxGroup *group);
    void onNanoDisplay() override;

private:
    CheckboxGroup *tags, *archives;
};

END_NAMESPACE_DISTRHO

#endif