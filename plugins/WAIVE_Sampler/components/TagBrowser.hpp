#ifndef TAG_BROWSER_HPP_INCLUDED
#define TAG_BROWSER_HPP_INCLUDED

#include "WidgetGroup.hpp"

#include "fonts.h"
#include "TagMap.hpp"
#include "SimpleButton.hpp"

START_NAMESPACE_DISTRHO

class TagBrowser
    : public WidgetGroup,
      public Button::Callback,
      public TagMap::Callback
{
public:
    explicit TagBrowser(WAIVEWidget *parent, SampleDatabase *sd_);

    void setCallback(TagMap::Callback *cb);

protected:
    void onNanoDisplay() override;
    void buttonClicked(Button *btn) override;
    void tagMapUpdated(TagMap *map) override;

private:
    SampleDatabase *sd;

    TagMap *tagMap;
    Button *selectAllBtn, *selectNoneBtn;

    DISTRHO_LEAK_DETECTOR(TagBrowser);
};

END_NAMESPACE_DISTRHO

#endif