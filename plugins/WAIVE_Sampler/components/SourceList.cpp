#include "SourceList.hpp"

START_NAMESPACE_DISTRHO

SourceList::SourceList(Widget *widget)
    : WAIVEWidget(widget),
      margin(5.0f),
      padding(5.f)
{
    loadSharedResources();
}

void SourceList::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    float rowWidth = width - 2.f * padding;
    float rowHeight = 2.f * font_size;

    beginPath();
    fillColor(background_color);
    rect(0, 0, width, height);
    fill();
    stroke();
    closePath();

    float y = 0.f;
    float x = padding;

    for (int i = 0; i < source_list.size(); i++)
    {
        y = i * (rowHeight + margin) + padding - scrollPos;

        if (y > height || y < -rowHeight)
            continue;
        drawSourceInfo(source_list[i], x, y, rowWidth, rowHeight);
    }
}

void SourceList::drawSourceInfo(const std::string &info, float x, float y, float width, float height)
{
    translate(x, y);

    beginPath();
    strokeColor(stroke_color);
    roundedRect(0, 0, width, height, 4.f);
    stroke();
    closePath();

    beginPath();
    fillColor(Color(30, 30, 30));
    fontSize(font_size);
    textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_LEFT);
    text(10.f, height / 2.0f, info.c_str(), nullptr);
    closePath();

    resetTransform();
}

bool SourceList::onScroll(const ScrollEvent &ev)
{
    if (!contains(ev.pos))
        return false;

    scrollPos += ev.delta.getY() * 10;
    scrollPos = std::max(scrollPos, 0.f);

    repaint();

    return true;
}

bool SourceList::onMotion(const MotionEvent &ev)
{
    return false;
}

bool SourceList::onMouse(const MouseEvent &ev)
{
    return false;
}

END_NAMESPACE_DISTRHO