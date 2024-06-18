#ifndef SOURCE_BROWSER_ROOT_HPP_INCLUDED
#define SOURCE_BROWSER_ROOT_HPP_INCLUDED

#include "NanoVG.hpp"

START_NAMESPACE_DISTRHO

class SourceBrowserRoot : public NanoStandaloneWindow
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void sourceBrowserClosed() = 0;
    };
    SourceBrowserRoot(Application &app, float width, float height, int flags = CREATE_ANTIALIAS);

    void setCallback(Callback *cb);

protected:
    void onNanoDisplay() override{};
    bool onClose() override;

private:
    Callback *callback;
};

END_NAMESPACE_DISTRHO

#endif