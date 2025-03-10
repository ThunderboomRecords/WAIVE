#include "SampleBrowser.hpp"

SampleBrowser::SampleBrowser(WAIVEWidget *widget, SampleDatabase *sd_, DragDropManager *manager)
    : WidgetGroup(widget),
      sd(sd_)
{
    float width = widget->getWidth();
    float height = widget->getHeight();

    sampleMap = new SampleMap(widget, manager);
    sampleMap->setSize(width - 4.0f, height - widget->getFontSize() * 2.f - 4.f, true);
    sampleMap->onTop(widget, Widget_Align::START, Widget_Align::START, 2.f, widget->getFontSize() * 2.f);
    sampleMap->background_color = WaiveColors::grey2;
    sampleMap->allSamples = &sd->fAllSamples;
    sampleMap->description = "Hover over sample to preview. Drag sample to Player on the right, or right click sample to choose Player slot.";
    addChildWidget(sampleMap);

    importSampleBtn = new Button(widget);
    importSampleBtn->setLabel("Import sample");
    importSampleBtn->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    importSampleBtn->resizeToFit();
    importSampleBtn->onTop(sampleMap, Widget_Align::END, Widget_Align::START);
    importSampleBtn->background_color = WaiveColors::grey1;
    importSampleBtn->setCallback(sampleMap);
    importSampleBtn->description = "Import sample and add to Sample Map.";
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
    previewToggle->setFont("Poppins-Light", Poppins_Light, Poppins_Light_len);
    previewToggle->resizeToFit();
    previewToggle->background_color = WaiveColors::grey1;
    previewToggle->accent_color = WaiveColors::light2;
    previewToggle->onTop(sampleMap, Widget_Align::START, Widget_Align::START);
    previewToggle->setCallback(this);
    previewToggle->description = "Enable/Disable sample preview on hover.";
    addChildWidget(previewToggle);

    sd->databaseUpdate += Poco::delegate(this, &SampleBrowser::onDatabaseChanged);
}

void SampleBrowser::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    strokeColor(accent_color);
    rect(0, 0, width, height);
    stroke();
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

void SampleBrowser::buttonClicked(Button *btn)
{
    if (btn == previewToggle)
        sampleMap->preview = previewToggle->getToggled();
};

void SampleBrowser::resizeWidgets()
{
    sampleMap->setSize(getWidth(), getHeight(), true);
    sampleMap->onTop(this);

    importSampleBtn->onTop(sampleMap, Widget_Align::END, Widget_Align::START);
    loading->leftOf(importSampleBtn);
    previewToggle->onTop(sampleMap, Widget_Align::START, Widget_Align::START);
}

void SampleBrowser::setCallback(SampleMap::Callback *cb)
{
    sampleMap->setCallback(cb);
}
