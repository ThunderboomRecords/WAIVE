#include "SampleBrowser.hpp"

SampleBrowser::SampleBrowser(WAIVEWidget *widget, SampleDatabase *sd_)
    : WidgetGroup(widget),
      sd(sd_)
{
    float width = widget->getWidth();
    float height = widget->getHeight();

    sampleMap = new SampleMap(widget);
    sampleMap->setSize(width - 4.0f, height - widget->getFontSize() * 2.f - 4.f, true);
    sampleMap->allSamples = &sd->fAllSamples;
    sampleMap->onTop(widget, Widget_Align::START, Widget_Align::START, 2.f, widget->getFontSize() * 2.f);
    addChildWidget(sampleMap);

    importSampleBtn = new Button(widget);
    importSampleBtn->setLabel("Import sample");
    importSampleBtn->resizeToFit();
    importSampleBtn->onTop(sampleMap, Widget_Align::END, Widget_Align::START);
    importSampleBtn->setCallback(sampleMap);
    addChildWidget(importSampleBtn);

    loading = new Spinner(widget);
    loading->setSize(importSampleBtn->getHeight(), importSampleBtn->getHeight());
    loading->leftOf(importSampleBtn);
    loading->setLoading(false);
    addChildWidget(loading);

    previewToggle = new Button(widget);
    previewToggle->isToggle = true;
    previewToggle->setToggled(true);
    previewToggle->setLabel("Preview");
    previewToggle->resizeToFit();
    previewToggle->onTop(sampleMap, Widget_Align::START, Widget_Align::START);
    previewToggle->setCallback(this);
    addChildWidget(previewToggle);

    sd->databaseUpdate += Poco::delegate(this, &SampleBrowser::onDatabaseChanged);
}

void SampleBrowser::onNanoDisplay()
{
    // const float width = getWidth();
    // const float height = getHeight();

    // beginPath();
    // fillColor(WaiveColors::grey1);
    // rect(0, 0, width, height);
    // fill();
    // closePath();
}

void SampleBrowser::onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg)
{
    switch (arg)
    {
    case SampleDatabase::DatabaseUpdate::SAMPLE_LIST_LOADED:
    case SampleDatabase::DatabaseUpdate::SAMPLE_ADDED:
    case SampleDatabase::DatabaseUpdate::SAMPLE_DELETED:
    case SampleDatabase::DatabaseUpdate::SAMPLE_UPDATED:
        break;
    default:
        return;
    }

    repaint();
}

void SampleBrowser::buttonClicked(Button *btn)
{
    if (btn == previewToggle)
        sampleMap->preview = previewToggle->getToggled();
};

void SampleBrowser::setCallback(SampleMap::Callback *cb)
{
    sampleMap->setCallback(cb);
}
