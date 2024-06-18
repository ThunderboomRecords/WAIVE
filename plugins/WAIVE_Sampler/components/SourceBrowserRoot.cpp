#include "SourceBrowserRoot.hpp"

START_NAMESPACE_DISTRHO

SourceBrowserRoot::SourceBrowserRoot(Application &app, float width, float height, int flags)
    : NanoStandaloneWindow(app, flags), callback(nullptr)
{
    setGeometryConstraints(width, height, true, false);
}

bool SourceBrowserRoot::onClose()
{
    printf("SourceBrowserRoot::onClose\n");
    if (callback != nullptr)
        callback->sourceBrowserClosed();

    return true;
}

void SourceBrowserRoot::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO