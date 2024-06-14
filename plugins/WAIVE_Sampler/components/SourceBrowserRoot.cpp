#include "SourceBrowserRoot.hpp"

START_NAMESPACE_DISTRHO

SourceBrowserRoot::SourceBrowserRoot(Application &app, float width, float height, int flags)
    : NanoStandaloneWindow(app, flags)
{
    setGeometryConstraints(width, height, true, false);
}

END_NAMESPACE_DISTRHO