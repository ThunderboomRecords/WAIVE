#ifndef BEATGRID_HPP_INCLUDED
#define BEATGRID_HPP_INCLUDED

#include "Window.hpp"
#include "Widget.hpp"
#include "NanoVG.hpp"
#include <iostream>

START_NAMESPACE_DISTRHO

#define INS 5
#define GS 16

class BeatGrid : public NanoSubWidget
{
public:
    explicit BeatGrid(Widget *widget) noexcept;

protected:
    void onNanoDisplay() override;
    bool onMouse(const MouseEvent &) override;
    bool onMotion(const MotionEvent &) override;
    bool onScroll(const ScrollEvent &) override;

private:
    uint8_t (*pattern)[INS][GS];
    uint8_t sixteenth;

    DGL::Color cellBackground;
    DGL::Color cellBorder;
    DGL::Color triggerBackground;
    DGL::Color triggerForeground;
    DGL::Color triggerHighlightBackground;
    DGL::Color triggerHighlightForeground;

    FontId _font;

    const char* channelNames[5] = {"kick", "snare", "hihat", "clap", "tom"};

    DISTRHO_LEAK_DETECTOR(BeatGrid);

    friend class AITestUI;
};


END_NAMESPACE_DISTRHO
#endif