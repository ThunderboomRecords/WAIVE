#include "SampleBrowser.hpp"

SampleBrowser::SampleBrowser(Window &window, SampleDatabase *sd_)
    : NanoTopLevelWidget(window),
      sd(sd_)
{
    loadSharedResources();

    float width = window.getWidth();
    float height = window.getHeight();

    sampleMap = new SampleMap(this);
    sampleMap->setSize(width, height, false);
    sampleMap->allSamples = &sd->fAllSamples;
    // sampleMap->selectedSample = &sd->

    importSampleBtn = new Button(this);
    importSampleBtn->setLabel("Import sample");
    importSampleBtn->resizeToFit();
    importSampleBtn->onTop(sampleMap, END, START);
    importSampleBtn->setCallback(sampleMap);

    loading = new Spinner(this);
    loading->setSize(importSampleBtn->getHeight(), importSampleBtn->getHeight());
    loading->leftOf(importSampleBtn);
    loading->setLoading(false);

    sd->databaseUpdate += Poco::delegate(this, &SampleBrowser::onDatabaseChanged);
}

void SampleBrowser::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(WaiveColors::grey1);
    rect(0, 0, width, height);
    fill();
    closePath();
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

void SampleBrowser::setCallback(SampleMap::Callback *cb)
{
    // callback = cb;
    sampleMap->setCallback(cb);
}
