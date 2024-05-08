#include "Waveform.hpp"

START_NAMESPACE_DISTRHO

Waveform::Waveform(Widget *widget) noexcept
    : NanoSubWidget(widget),
      backgroundColor(Color(40, 40, 40)),
      lineColor(Color(200, 200, 200)),
      waveformCached(false),
      dragAction(NONE),
      waveformSelectStart(0),
      waveformSelectEnd(0),
      selectable(false),
      zoomable(true),
      visibleStart(0),
      visibleEnd(100),
      reduced(false),
      wf(nullptr)
{
}

void Waveform::setWaveform(std::vector<float> *wf_)
{
    waveformCached = false;
    wf = wf_;
}

void Waveform::setSelection(int start, bool sendCallback = false)
{
    waveformSelectStart = std::clamp(start, 0, *waveformLength);

    if (callback != nullptr && sendCallback)
        callback->waveformSelection(this, waveformSelectStart);

    repaint();
}

void Waveform::waveformNew()
{
    if (wf == nullptr)
    {
        std::cout << "waveform not set\n";
        return;
    }

    if (*waveformLength == 0)
        return;

    visibleStart = 0;
    visibleEnd = *waveformLength;

    calculateWaveform();
}

void Waveform::waveformUpdated()
{
    visibleStart = 0;
    visibleEnd = *waveformLength;
    calculateWaveform();
}

void Waveform::calculateWaveform()
{
    waveformCached = false;

    const int width = getWidth();
    double samples_per_pixel = (visibleEnd - visibleStart) / (double)(width);
    reduced = samples_per_pixel > 1.0f;

    waveformMin.resize(width);
    waveformMax.resize(width);

    for (int i = 0; i < width; i++)
    {
        waveformMin[i] = 0.0f;
        waveformMax[i] = 0.0f;

        int start = (int)(i * samples_per_pixel) + visibleStart;
        int end = std::min((int)*waveformLength, (int)((i + 1) * samples_per_pixel) + visibleStart);

        auto [min, max] = std::minmax_element(
            wf->begin() + start,
            wf->begin() + end);

        waveformMin[i] = min[0];
        waveformMax[i] = max[0];

        waveformMax[i] = std::clamp(waveformMax[i], -1.0f, 1.0f);
        waveformMin[i] = std::clamp(waveformMin[i], -1.0f, 1.0f);
    }

    waveformCached = true;
    repaint();
}

void Waveform::onNanoDisplay()
{
    if (!isVisible())
        return;

    const int width = getWidth();
    const int height = getHeight();
    const int half = height / 2;

    beginPath();
    fillColor(backgroundColor);
    rect(0, 0, width, height);
    fill();
    closePath();

    if (!waveformCached || wf == nullptr || *waveformLength == 0)
        return;

    // draw waveform
    beginPath();
    lineJoin(BEVEL);
    lineCap(BUTT);
    strokeColor(lineColor);
    strokeWidth(1.0f);
    moveTo(0, half);

    for (int i = 0; i < waveformMin.size(); i++)
    {

        lineTo(i, half - waveformMax[i] * half);
        if (reduced)
            lineTo(i, half - waveformMin[i] * half);
    }
    stroke();
    closePath();

    //  draw highlighted region
    if (selectable)
    {
        float cursorPosStart = (float)(waveformSelectStart - visibleStart) / (visibleEnd - visibleStart);
        float x1 = width * std::clamp(cursorPosStart, 0.0f, 1.0f);
        beginPath();
        fillColor(Color(255, 200, 0, 0.8f));
        rect(x1, 0, 2, height);
        fill();
        closePath();
    }

    // draw minimap if zoomed in
    if (visibleStart > 0 || visibleEnd < *waveformLength)
    {
        beginPath();
        fillColor(Color(30, 30, 30));
        rect(0, height - 12.0f, width, 12.0f);
        fill();
        closePath();

        float x;
        if (selectable)
        {
            beginPath();
            fillColor(Color(200, 200, 0, 0.3f));
            x = (float)waveformSelectStart / *waveformLength * width;
            // float w = (float)(waveformSelectEnd - waveformSelectStart) / *waveformLength * width;
            rect(x, height - 12.0f, 2, 12.0f);
            fill();
            closePath();
        }

        beginPath();
        strokeColor(Color(230, 230, 230));
        x = (float)visibleStart / *waveformLength * width;
        float w = (float)(visibleEnd - visibleStart) / *waveformLength * width;
        rect(x + 1.0f, height - 12.0f + 1.0f, w - 2.0f, 12.0f - 1.0f);
        stroke();
        closePath();
    }
}

bool Waveform::onMouse(const MouseEvent &ev)
{
    if (!isVisible())
        return false;

    if (contains(ev.pos) && ev.press)
    {
        dragAction = CLICKING;
        clickStart = ev.pos;
        return true;
    }
    else if (!ev.press && dragAction != NONE)
    {
        int range = visibleEnd - visibleStart;
        int cursorPos = (int)(range * (ev.pos.getX() / getWidth()));
        cursorPos = std::clamp(cursorPos, 0, (int)range);
        switch (dragAction)
        {
        case SELECTING:

            break;
        case SCROLLING:
            break;

        case CLICKING:
            if (!selectable)
                break;
            waveformSelectStart = cursorPos + visibleStart;
            if (*waveformLength > 0)
                callback->waveformSelection(this, waveformSelectStart);
            break;

        default:
            break;
        }
        dragAction = NONE;

        repaint();
        return true;
    }
    else
    {
        return false;
    }
}

bool Waveform::onMotion(const MotionEvent &ev)
{
    if (dragAction == NONE)
        return false;

    if (dragAction == CLICKING)
    {
        if (ev.pos.getY() <= getHeight() - 12.0f)
        {
            if (!selectable)
                return true;
            // started selecting
            dragAction = SELECTING;
            int range = visibleEnd - visibleStart;
            waveformSelectStart = visibleStart + (int)(range * ev.pos.getX() / getWidth());
            waveformSelectEnd = waveformSelectStart;
        }
        else
        {
            // started scrolling
            dragAction = SCROLLING;
        }
    }

    int range = visibleEnd - visibleStart;

    int cursorPos = (int)(range * (ev.pos.getX() / getWidth()));
    cursorPos = std::clamp(cursorPos, 0, (int)range);

    float dX;
    int dV;

    switch (dragAction)
    {
    case SELECTING:
        break;
    case SCROLLING:
        dX = ev.pos.getX() - clickStart.getX();
        dV = (int)(dX * (*waveformLength / getWidth()));

        if (visibleStart + dV <= 0)
            dV = -visibleStart;

        if (visibleEnd + dV >= *waveformLength)
            dV = *waveformLength - visibleEnd;

        visibleEnd += dV;
        visibleStart += dV;

        clickStart = ev.pos;

        calculateWaveform();
        break;
    default:
        break;
    }

    repaint();
    return true;
}

bool Waveform::onScroll(const ScrollEvent &ev)
{
    if (!contains(ev.pos) || !zoomable || wf == nullptr || wf->size() == 0)
        return false;

    int range = visibleEnd - visibleStart;
    int newRange = range * (1.0f + ev.delta.getY() * 0.05f);
    newRange = std::max(newRange, (int)getWidth());

    visibleEnd = visibleStart + newRange;
    if (visibleEnd >= *waveformLength)
    {
        int delta = visibleEnd - *waveformLength;
        visibleEnd -= delta;
        visibleStart -= delta;
    }

    if (visibleStart < 0)
        visibleStart = 0;

    calculateWaveform();
    return true;
}

void Waveform::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO