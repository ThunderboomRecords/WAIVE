#include "SampleBrowserRoot.hpp"

START_NAMESPACE_DISTRHO

SampleBrowserRoot::SampleBrowserRoot(Application &app, float width, float height, int flags)
    : NanoStandaloneWindow(app, flags)
{
    setGeometryConstraints(width, height, true, false);
}

bool SampleBrowserRoot::onClose()
{
    // printf("SampleBrowserRoot::onClose\n");

    return true;
}

END_NAMESPACE_DISTRHO