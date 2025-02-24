#ifndef SAMPLE_MAP_HPP_INCLUDED
#define SAMPLE_MAP_HPP_INCLUDED

#include <map>
#include <iostream>

#include "WAIVEWidget.hpp"

#include "Menu.hpp"
#include "DragDrop.hpp"
#include "SimpleButton.hpp"
#include "SampleDatabase.hpp"

START_NAMESPACE_DISTRHO

class SampleMap : public WAIVEWidget,
                  public DragDropWidget,
                  public Menu::Callback,
                  public Button::Callback
{
public:
    class Callback
    {
    public:
        virtual ~Callback() {};
        virtual void mapSampleHovered(int id) = 0;
        virtual void mapSampleSelected(int id) = 0;
        virtual void mapSampleLoadSlot(int index, int slot) = 0;
        virtual void mapSampleImport() = 0;
        virtual void mapSampleDelete(int id) = 0;
    };

    explicit SampleMap(Widget *widget, DragDropManager *manager) noexcept;
    void setCallback(Callback *cb);

    std::vector<std::shared_ptr<SampleInfo>> *allSamples;
    std::shared_ptr<SampleInfo> *selectedSample;
    int highlightSample, contextMenuSample;
    bool preview;

    Menu *menu;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;
    void onMenuItemSelection(Menu *menu, int item, const std::string &value) override;
    void buttonClicked(Button *btn) override;

    void dataAccepted(DragDropWidget *destination) override;
    void dataRejected(DragDropWidget *destination) override;

private:
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