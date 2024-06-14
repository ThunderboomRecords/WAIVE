#ifndef SOURCE_BROWSER_ROOT_HPP_INCLUDED
#define SOURCE_BROWSER_ROOT_HPP_INCLUDED

#include "NanoVG.hpp"

START_NAMESPACE_DISTRHO

class SourceBrowserRoot : public NanoStandaloneWindow
{
public:
    SourceBrowserRoot(Application &app, float width, float height, int flags = CREATE_ANTIALIAS);

protected:
    void onNanoDisplay() override{};
};

END_NAMESPACE_DISTRHO

#endif