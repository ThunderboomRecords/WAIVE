#ifndef SAMPLE_BROWSER_HPP_INCLUDED
#define SAMPLE_BROWSER_HPP_INCLUDED

// #include "NanoVG.hpp"
// #include "Window.hpp"

#include "WidgetGroup.hpp"

#include "Label.hpp"
#include "Spinner.hpp"
#include "SampleMap.hpp"
#include "SimpleButton.hpp"

START_NAMESPACE_DISTRHO

class SampleBrowser
    : public WidgetGroup
{
public:
    explicit SampleBrowser(WAIVEWidget *parent, SampleDatabase *sd_);

    void setCallback(SampleMap::Callback *cb);
    void onDatabaseChanged(const void *pSender, const SampleDatabase::DatabaseUpdate &arg);

    Spinner *loading;

protected:
    void onNanoDisplay() override;

private:
    SampleDatabase *sd;

    SampleMap *sampleMap;
    Button *importSampleBtn;

    DISTRHO_LEAK_DETECTOR(SampleBrowser);
};

END_NAMESPACE_DISTRHO

#endif