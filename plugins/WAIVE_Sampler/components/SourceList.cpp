#include "SourceList.hpp"

START_NAMESPACE_DISTRHO

SourceList::SourceList(Widget *widget)
    : WAIVEWidget(widget),
      margin(5.0f),
      padding(5.f),
      rowHeight(30.f),
      scrollBarWidth(8),
      scrolling(false)
{
    loadSharedResources();
}

void SourceList::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    float rowWidth = width - 2.f * padding - scrollBarWidth;
    rowHeight = 2.f * font_size;

    int maxDisplay = height / (rowHeight + margin);
    int n = source_list.size();

    // update scrollPos incase number of items has changed
    clampScrollPos();

    beginPath();
    fillColor(background_color);
    rect(0, 0, width, height);
    fill();
    stroke();
    closePath();

    float y = 0.f;
    float x = padding;

    for (int i = 0; i < n; i++)
    {
        y = i * (rowHeight + margin) + padding - scrollPos;

        if (y > height || y < -rowHeight)
            continue;
        drawSourceInfo(source_list[i], x, y, rowWidth, rowHeight);
    }

    // scroll bar
    if (n > maxDisplay)
    {
        float steps = height / n;
        beginPath();
        fillColor(stroke_color);
        rect(
            width - scrollBarWidth,
            (1 + scrollPos / rowHeight) * steps,
            scrollBarWidth,
            steps * maxDisplay);
        fill();
        closePath();
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

    scrollPos -= ev.delta.getY() * 10;
    clampScrollPos();

    repaint();

    return true;
}

void SourceList::clampScrollPos()
{
    scrollPos = std::max(scrollPos, 0.f);
    scrollPos = std::min(scrollPos, (source_list.size() + 2) * rowHeight - getHeight());
}

bool SourceList::onMotion(const MotionEvent &ev)
{
    if (!isVisible())
        return false;

    if (scrolling)
    {
        scrollPos = (source_list.size() * rowHeight) * ev.pos.getY() / getHeight();
        clampScrollPos();

        repaint();

        return true;
    }

    return false;
}

bool SourceList::onMouse(const MouseEvent &ev)
{
    if (!isVisible())
        return false;

    if (!scrolling && ev.press && contains(ev.pos))
    {
        if (ev.pos.getX() > getWidth() - scrollBarWidth)
        {
            scrolling = true;
            return true;
        }
    }
    else if (scrolling && !ev.press)
        scrolling = false;

    return false;
}

END_NAMESPACE_DISTRHO