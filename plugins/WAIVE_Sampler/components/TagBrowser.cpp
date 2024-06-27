#include "TagBrowser.hpp"

TagBrowser::TagBrowser(Window &window, SampleDatabase *sd_)
    : NanoTopLevelWidget(window),
      sd(sd_)
{
    loadSharedResources();

    float width = window.getWidth();
    float height = window.getHeight();

    tagMap = new TagMap(this);
    tagMap->setSize(width, height, false);
    tagMap->setFont("VG5000", VG5000, VG5000_len);
    tagMap->tagList = &sd->tagList;
    tagMap->setCallback(this);

    select_none = new Button(this);
    select_none->setLabel("None");
    select_none->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    select_none->resizeToFit();
    select_none->setCallback(this);
    select_none->onTop(tagMap, END, START, 10);

    select_all = new Button(this);
    select_all->setLabel("All");
    select_all->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    select_all->resizeToFit();
    select_all->setCallback(this);
    select_all->leftOf(select_none, START, 10);
}

void TagBrowser::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(WaiveColors::grey1);
    rect(0, 0, width, height);
    fill();
    closePath();
}

void TagBrowser::buttonClicked(Button *btn)
{
    if (btn == select_all)
        tagMap->setSelectAll(true);
    else if (btn == select_none)
        tagMap->setSelectAll(false);
}

void TagBrowser::tagMapUpdated(TagMap *map)
{
    std::string tagSubset = map->getSelectedTagList();
    sd->filterConditions.tagIn.assign(tagSubset);
    sd->filterSources();
}

void TagBrowser::setCallback(TagMap::Callback *cb)
{
    tagMap->setCallback(cb);
}
