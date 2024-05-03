#include "Waveform.hpp"

START_NAMESPACE_DISTRHO

Waveform::Waveform(Widget *widget) noexcept
    : NanoSubWidget(widget),
      backgroundColor(Color(40, 40, 40)),
      lineColor(Color(200, 200, 200)),
      waveformCached(false),
      dragging(false),
      waveformLength(0),
      waveformSelectStart(0),
      waveformSelectEnd(0),
      selectable(false),
      zoomLevel(1.0f),
      visibleStart(0),
      visibleEnd(100),
      visibleStartCached(1),
      visibleEndCached(101)
{
}

void Waveform::setWaveform(std::vector<float> *wf_)
{
    waveformCached = false;
    wf = wf_;
}

void Waveform::waveformNew()
{
    waveformLength = wf->size();

    if (waveformLength == 0)
        return;

    visibleStart = 0;
    visibleEnd = waveformLength;
    zoomLevel = 1.0f;

    calculateWaveform();
}

void Waveform::calculateWaveform()
{
    // if(visibleEnd == visibleEndCached && visibleStart == visibleStartCached)
    //     return;

    waveformCached = false;

    const int width = getWidth();
    double samples_per_pixel = (visibleEnd - visibleStart) / (double)(width);

    waveformMin.resize(width);
    waveformMax.resize(width);

    float fIndex = 0.0f;
    int index = 0;

    for (int i = 0; i < width; i++)
    {
        waveformMin[i] = 0.0f;
        waveformMax[i] = 0.0f;
        for (int j = 0; j < samples_per_pixel; j++)
        {
            try
            {
                waveformMax[i] = std::max(waveformMax[i], wf->at(visibleStart + index));
                waveformMin[i] = std::min(waveformMin[i], wf->at(visibleStart + index));
                index++;
            }
            catch (const std::out_of_range &e)
            {
                // std::cout << index << " out of range" << std::endl;
                break;
            }
        }

        waveformMax[i] = std::clamp(waveformMax[i], -1.0f, 1.0f);
        waveformMin[i] = std::clamp(waveformMin[i], -1.0f, 1.0f);
    }

    waveformCached = true;
    visibleStartCached = visibleStart;
    visibleEndCached = visibleEnd;
    repaint();
}

void Waveform::onNanoDisplay()
{
    const int width = getWidth();
    const int height = getHeight();
    const int half = height / 2;

    beginPath();
    fillColor(backgroundColor);
    rect(0, 0, width, height);
    fill();
    closePath();

    if (!waveformCached)
        return;

    beginPath();
    lineCap(ROUND);
    strokeColor(lineColor);
    strokeWidth(1.0f);
    moveTo(0, half);

    for (int i = 0; i < waveformMin.size(); i++)
    {
        float min = waveformMin[i];
        float max = waveformMax[i];

        moveTo(i, half - max * half);
        lineTo(i, half - min * half);
    }
    stroke();
    closePath();

    if (waveformLength > 0)
    {
        float cursorPosStart = (float)waveformSelectStart / waveformLength;
        float cursorPosEnd = (float)waveformSelectEnd / waveformLength;
        float x1 = width * cursorPosStart;
        float x2 = width * cursorPosEnd;
        beginPath();
        fillColor(Color(200, 200, 0, 0.3f));
        rect(x1, 0, x2 - x1, height);
        fill();
        closePath();
    }
}

bool Waveform::onMouse(const MouseEvent &ev)
{
    if (!selectable)
        return false;

    if (contains(ev.pos) && ev.press)
    {
        dragging = true;
        waveformSelectStart = (uint)(waveformLength * ev.pos.getX() / getWidth());
        waveformSelectEnd = waveformSelectStart;
    }
    else if (!ev.press && dragging)
    {
        dragging = false;

        if (waveformSelectStart != waveformSelectEnd && waveformLength > 0)
        {
            callback->waveformSelection(this, waveformSelectStart, waveformSelectEnd);
        }

        repaint();
    }
    else
    {
        return false;
    }

    return true;
}

bool Waveform::onMotion(const MotionEvent &ev)
{

    if (!dragging || !selectable)
        return false;

    int cursorPos = (int)(waveformLength * ev.pos.getX() / getWidth());
    cursorPos = std::clamp(cursorPos, 0, (int)waveformLength);

    if (cursorPos < waveformSelectStart)
    {
        waveformSelectStart = cursorPos;
    }
    else if (cursorPos > waveformSelectStart)
    {
        waveformSelectEnd = cursorPos;
    }

    repaint();
    return true;
}

bool Waveform::onScroll(const ScrollEvent &ev)
{
    if (!contains(ev.pos))
        return false;

    zoomLevel *= (1.0f + ev.delta.getY() * 0.05f);
    zoomLevel = std::min(zoomLevel, 1.0f);

    // std::cout << zoomLevel << " " << ev.delta.getY() << " x: " << ev.delta.getX() << std::endl;

    visibleEnd = waveformLength * zoomLevel;
    visibleEnd = std::clamp(visibleEnd, visibleStart + 10, waveformLength);

    calculateWaveform();
    return true;
}

void Waveform::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO