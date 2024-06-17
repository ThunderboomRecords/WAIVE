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

    if (n < maxDisplay)
        scrollPos = 0.f;
    else
        clampScrollPos();

    beginPath();
    fillColor(background_color);
    rect(0, 0, width, height);
    fill();
    stroke();
    closePath();

    if (n == 0)
    {
        beginPath();
        fillColor(text_color);
        fontSize(font_size);
        textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_CENTER);
        text(width / 2.f, height / 2.f, "no results found", nullptr);
        closePath();
        return;
    }

    float y = 0.f;
    float x = padding;

    int startIndex = scrollPos / (rowHeight + margin);

    for (int i = startIndex; i < startIndex + maxDisplay + 2; i++)
    {
        y = i * (rowHeight + margin) + padding - scrollPos;

        if (y < -rowHeight)
            continue;
        if (y > height || i >= n)
            break;

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
            (scrollPos / (rowHeight + margin)) * steps,
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
    fillColor(text_color);
    moveTo(8, 10);
    lineTo(8, height - 10);
    lineTo(23, height / 2);
    fill();
    closePath();

    beginPath();
    fillColor(text_color);
    fontSize(font_size);
    textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_LEFT);
    text(30.f, height / 2.0f, info.c_str(), nullptr);
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
    scrollPos = std::min(scrollPos, (source_list.size()) * (rowHeight + margin) + 2 * padding - getHeight());
    scrollPos = std::max(scrollPos, 0.f);
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