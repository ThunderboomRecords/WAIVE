#ifndef SOURCE_LIST_HPP_INCLUDED
#define SOURCE_LIST_HPP_INCLUDED

#include "WAIVEWidget.hpp"
#include "WAIVEImage.hpp"
#include "SampleDatabase.hpp"

#include "download_icon.h"

START_NAMESPACE_DISTRHO

class SourceList : public WAIVEWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void sourceDownload(int index) = 0;
    };

    SourceList(Widget *widget);
    void setCallback(Callback *cb);

    std::vector<SourceInfo> *source_info;
    std::mutex *source_info_mtx;

    int scrollBarWidth;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;

    float margin, padding;

private:
    Callback *callback;
    void clampScrollPos();
    float scrollPos;
    float rowHeight;
    void drawSourceInfo(const SourceInfo &info, float x, float y, float width, float height, bool highlight);

    WAIVEImage *download;

    bool scrolling;
    int highlighting;
};

END_NAMESPACE_DISTRHO

#endif