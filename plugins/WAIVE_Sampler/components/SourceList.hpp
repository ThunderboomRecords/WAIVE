#ifndef SOURCE_LIST_HPP_INCLUDED
#define SOURCE_LIST_HPP_INCLUDED

#include "WAIVEWidget.hpp"
#include "SampleDatabase.hpp"

START_NAMESPACE_DISTRHO

class SourceList : public WAIVEWidget
{
public:
    SourceList(Widget *widget);

    std::vector<std::string> source_list;

    int scrollBarWidth;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;

    float margin, padding;

private:
    void clampScrollPos();
    float scrollPos;
    float rowHeight;
    void drawSourceInfo(const std::string &info, float x, float y, float width, float height);

    bool scrolling;
};

END_NAMESPACE_DISTRHO

#endif