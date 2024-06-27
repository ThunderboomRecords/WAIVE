#ifndef TAG_BROWSER_HPP_INCLUDED
#define TAG_BROWSER_HPP_INCLUDED

#include "NanoVG.hpp"
#include "Window.hpp"

#include "fonts.h"
#include "TagMap.hpp"
#include "SimpleButton.hpp"

START_NAMESPACE_DISTRHO

class TagBrowser
    : public NanoTopLevelWidget,
      public Button::Callback,
      public TagMap::Callback
{
public:
    explicit TagBrowser(Window &window, SampleDatabase *sd_);

    void setCallback(TagMap::Callback *cb);

protected:
    void onNanoDisplay() override;
    void buttonClicked(Button *btn) override;
    void tagMapUpdated(TagMap *map) override;

private:
    SampleDatabase *sd;

    TagMap *tagMap;
    Button *select_all, *select_none;

    DISTRHO_LEAK_DETECTOR(TagBrowser);
};

END_NAMESPACE_DISTRHO

#endif