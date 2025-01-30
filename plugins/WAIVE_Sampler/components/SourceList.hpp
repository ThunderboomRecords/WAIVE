#ifndef SOURCE_LIST_HPP_INCLUDED
#define SOURCE_LIST_HPP_INCLUDED

#include "WAIVEWidget.hpp"
#include "WAIVEImage.hpp"
#include "SampleDatabase.hpp"
#include "WAIVEUtils.hpp"

#include "Poco/Random.h"

#include "download_icon.h"

START_NAMESPACE_DISTRHO

class SourceList : public WAIVEWidget,
                   public IdleCallback
{
public:
    class Callback
    {
    public:
        virtual ~Callback() {};
        virtual void sourceDownload(int index) = 0;
        virtual void sourcePreview(int index, bool start) = 0;
        virtual void sourceLoad(int index) = 0;
    };

    SourceList(Widget *widget);
    void setCallback(Callback *cb);
    void selectRandom();
    void computeColumnWidths();

    std::vector<SourceInfo> *source_info;
    std::mutex *source_info_mtx;
    int previewPlaying;

    int scrollBarWidth;
    std::string info;
    Color scrollGutter, scrollHandle;
    float margin, padding;
    int selected;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;
    void idleCallback() override;

private:
    Callback *callback;
    void clampScrollPos();
    float scrollPos;
    float rowHeight;
    float columnLabel, columnLicense, columnDownload;
    void drawSourceInfo(const SourceInfo &info, float x, float y, float width, float height, bool highlight, bool playing);

    WAIVEImage *download;

    bool scrolling;
    int highlighting;

    Poco::Random random;
};

END_NAMESPACE_DISTRHO

#endif