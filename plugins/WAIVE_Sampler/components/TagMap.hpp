#ifndef TAG_MAP_HPP_INCLUDED
#define TAG_MAP_HPP_INCLUDED

#include <map>
#include <iostream>

#include "WAIVEWidget.hpp"

#include "Menu.hpp"
#include "SimpleButton.hpp"
#include "SampleDatabase.hpp"

START_NAMESPACE_DISTRHO

class TagMap : public WAIVEWidget
{
public:
    class Callback
    {
    public:
        virtual ~Callback() {};
        virtual void tagMapUpdated(TagMap *map) {}
    };

    explicit TagMap(Widget *window) noexcept;
    void setCallback(Callback *cb);
    void setSelectAll(bool all);
    std::string getSelectedTagList();

    std::vector<Tag> *tagList;
    std::unordered_map<int, Tag *> selected;
    Tag *highlighted;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;

private:
    enum DragAction
    {
        NONE = 0,
        CLICKING,
        SELECTING,
        SCROLLING,
    };

    Color get2DColor(float x, float y) const;
    Callback *callback;

    Color c0, c1, c2, c3;

    Point<double> embeddingToMap(Point<double>);
    Point<double> mapToEmbedding(Point<double>);

    float zoomLevel;
    Point<double> centerPos, dragStart, centerStart, cursorPos;

    std::map<int, Color> colorCache;

    DragAction dragAction;

    DISTRHO_LEAK_DETECTOR(TagMap);
};

END_NAMESPACE_DISTRHO

#endif