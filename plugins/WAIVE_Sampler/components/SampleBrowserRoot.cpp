#include "SampleBrowserRoot.hpp"

START_NAMESPACE_DISTRHO

SampleBrowserRoot::SampleBrowserRoot(Application &app, float width, float height, int flags)
    : NanoStandaloneWindow(app, flags), callback(nullptr)
{
    setGeometryConstraints(width, height, true, false);
}

bool SampleBrowserRoot::onClose()
{
    printf("SampleBrowserRoot::onClose\n");
    // if (callback != nullptr)
    //     callback->sourceBrowserClosed();

    return true;
}

void SampleBrowserRoot::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO