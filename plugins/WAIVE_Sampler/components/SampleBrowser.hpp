#ifndef SAMPLE_BROWSER_HPP_INCLUDED
#define SAMPLE_BROWSER_HPP_INCLUDED

#include "WidgetGroup.hpp"

#include "Label.hpp"
#include "Spinner.hpp"
#include "SampleMap.hpp"
#include "SimpleButton.hpp"

START_NAMESPACE_DISTRHO

class SampleBrowser
    : public WidgetGroup,
      public Button::Callback
{
public:
    explicit SampleBrowser(WAIVEWidget *parent, SampleDatabase *sd_);

    void setCallback(SampleMap::Callback *cb);
    void onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg);
    void repositionWidgets();

    Spinner *loading;

protected:
    void onNanoDisplay() override;
    void buttonClicked(Button *btn) override;

private:
    SampleDatabase *sd;

    SampleMap *sampleMap;
    Button *importSampleBtn, *previewToggle;

    DISTRHO_LEAK_DETECTOR(SampleBrowser);
};

END_NAMESPACE_DISTRHO

#endif