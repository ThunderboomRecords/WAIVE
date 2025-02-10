#include "TagBrowser.hpp"

TagBrowser::TagBrowser(WAIVEWidget *widget, SampleDatabase *sd_)
    : WidgetGroup(widget),
      sd(sd_)
{
    float width = widget->getWidth();
    float height = widget->getHeight();

    tagMap = new TagMap(widget);
    tagMap->setSize(width - 4.0f, height - widget->getFontSize() * 2.f - 4.f, true);
    tagMap->onTop(widget, Widget_Align::START, Widget_Align::START, 2.f, widget->getFontSize() * 2.f);
    tagMap->setFont("VG5000", VG5000, VG5000_len);
    tagMap->tagList = &sd->tagList;
    tagMap->background_color = WaiveColors::grey2;
    tagMap->setCallback(this);
    addChildWidget(tagMap);

    selectNoneBtn = new Button(widget);
    selectNoneBtn->setLabel("None");
    selectNoneBtn->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    selectNoneBtn->resizeToFit();
    selectNoneBtn->setCallback(this);
    selectNoneBtn->onTop(tagMap, Widget_Align::END, Widget_Align::START, 10);
    selectNoneBtn->background_color = WaiveColors::grey1;
    selectNoneBtn->description = "Deselect all tags";
    addChildWidget(selectNoneBtn);

    selectAllBtn = new Button(widget);
    selectAllBtn->setLabel("All");
    selectAllBtn->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    selectAllBtn->resizeToFit();
    selectAllBtn->setCallback(this);
    selectAllBtn->leftOf(selectNoneBtn, Widget_Align::START, 10);
    selectAllBtn->background_color = WaiveColors::grey1;
    selectAllBtn->description = "Select all tags";
    addChildWidget(selectAllBtn);
}

void TagBrowser::onNanoDisplay()
{
    if (renderDebug)
    {
        beginPath();
        rect(0, 0, getWidth(), getHeight());
        strokeColor(accent_color);
        stroke();
        closePath();
    }
}

void TagBrowser::buttonClicked(Button *btn)
{
    if (btn == selectAllBtn)
        tagMap->setSelectAll(true);
    else if (btn == selectNoneBtn)
        tagMap->setSelectAll(false);
}

void TagBrowser::tagMapUpdated(TagMap *map)
{
    std::string tagSubset = map->getSelectedTagList();
    sd->filterConditions.tagIn.assign(tagSubset);
    sd->filterSources();
}

void TagBrowser::repositionWidgets()
{
    tagMap->setSize(getWidth(), getHeight(), true);
    tagMap->onTop(this);

    selectNoneBtn->onTop(tagMap, Widget_Align::END, Widget_Align::START, 10);
    selectAllBtn->leftOf(selectNoneBtn, Widget_Align::START, 10);
}

void TagBrowser::setCallback(TagMap::Callback *cb)
{
    tagMap->setCallback(cb);
}
