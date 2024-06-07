#ifndef SAMPLE_MAP_HPP_INCLUDED
#define SAMPLE_MAP_HPP_INCLUDED

#include <map>
#include <iostream>

#include "fonts.h"
#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"

#include "Menu.hpp"
#include "SampleDatabase.hpp"

START_NAMESPACE_DISTRHO

class SampleMap : public NanoSubWidget,
                  public Menu::Callback
{
public:
    class Callback
    {
    public:
        virtual ~Callback(){};
        virtual void mapSampleHovered(int id) = 0;
        virtual void mapSampleSelected(int id) = 0;
        virtual void mapSampleLoadSlot(int index, int slot) = 0;
    };

    explicit SampleMap(Widget *widget) noexcept;
    void setCallback(Callback *cb);

    Color background_color;

    std::vector<std::shared_ptr<SampleInfo>> *allSamples;
    std::shared_ptr<SampleInfo> *selectedSample;
    int highlightSample, contextMenuSample;

    Menu *menu;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;
    void onMenuItemSelection(Menu *menu, int item, const char *value) override;

private:
    enum DragAction
    {
        NONE = 0,
        CLICKING,
        SELECTING,
        SCROLLING,
    };

    Color get2DColor(float x, float y);
    Callback *callback;

    Color c0, c1, c2, c3;

    Point<double> embeddingToMap(Point<double>);
    Point<double> mapToEmbedding(Point<double>);

    float zoomLevel;
    Point<double> centerPos, dragStart, centerStart, cursorPos;

    std::map<int, Color> colorCache;

    DragAction dragAction;
};

END_NAMESPACE_DISTRHO

#endif