#ifndef SAMPLE_BROWSER_ROOT_HPP_INCLUDED
#define SAMPLE_BROWSER_ROOT_HPP_INCLUDED

#include "NanoVG.hpp"

START_NAMESPACE_DISTRHO

class SampleBrowserRoot : public NanoStandaloneWindow
{
public:
    SampleBrowserRoot(Application &app, float width, float height, int flags = CREATE_ANTIALIAS);

protected:
    void onNanoDisplay() override{};
    bool onClose() override;
};

END_NAMESPACE_DISTRHO

#endif