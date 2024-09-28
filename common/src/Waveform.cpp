#include "Waveform.hpp"

START_NAMESPACE_DISTRHO

Waveform::Waveform(Widget *widget) noexcept
    : WAIVEWidget(widget, 0), // No anti-aliasing to speed up rendering
      waveformCached(false),
      dragAction(NONE),
      waveformSelectStart(0),
      waveformSelectEnd(0),
      selectable(false),
      zoomable(true),
      visibleStart(0),
      visibleEnd(100),
      reduced(false),
      wf(nullptr),
      wfFeatures(nullptr),
      featureHighlight(-1),
      feature_color(WaiveColors::accent1),
      cursor_color(WaiveColors::accent2)
{
    level_of_detail = int(4 * scale_factor);
}

void Waveform::setWaveform(std::vector<float> *wf_)
{
    waveformCached = false;
    wf = wf_;
}

void Waveform::setWaveformFeatures(std::vector<WaveformFeature> *wfFeatures_)
{
    wfFeatures = wfFeatures_;
}

void Waveform::setSelection(int start, bool sendCallback = false)
{
    waveformSelectStart = std::clamp(start, 0, waveformLength);

    if (callback != nullptr && sendCallback)
        callback->waveformSelection(this, waveformSelectStart);

    repaint();
}

void Waveform::setWaveformLength(int length)
{
    waveformLength = length;
}

void Waveform::waveformNew()
{
    if (wf == nullptr)
        return;

    if (waveformLength == 0)
        return;

    visibleStart = 0;
    visibleEnd = waveformLength;

    calculateWaveform();
}

void Waveform::waveformUpdated()
{
    visibleStart = 0;
    visibleEnd = waveformLength;
    calculateWaveform();
}

void Waveform::calculateWaveform()
{
    waveformCached = false;

    if (waveformLength == 0)
        return;

    const int width = getWidth() - 2;
    int number_of_blocks = width / level_of_detail;

    if (number_of_blocks <= 0)
        return;

    double samples_per_block = (double)(visibleEnd - visibleStart) / number_of_blocks;
    reduced = samples_per_block > 2 * level_of_detail;

    if (waveformMin.size() != number_of_blocks)
    {
        waveformMin.resize(number_of_blocks);
        waveformMax.resize(number_of_blocks);
    }

    int start, end;
    for (int i = 0; i < number_of_blocks; i++)
    {
        start = (int)(i * samples_per_block) + visibleStart;
        end = std::min((int)waveformLength, (int)((i + 1) * samples_per_block) + visibleStart);

        if (start >= wf->size() || end > wf->size() || start >= end)
            continue;

        auto [min, max] = std::minmax_element(
            wf->begin() + start,
            wf->begin() + end);

        waveformMin[i] = *min;
        waveformMax[i] = *max;
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
    const int half = std::floor(height / 2);

    float range = visibleEnd - visibleStart;

    beginPath();
    fillColor(background_color);
    roundedRect(0, 0, width, height, 8.f);
    fill();
    closePath();

    if (!waveformCached || wf == nullptr || waveformLength == 0)
        return;

    // if (reduced)
    // {
    //     beginPath();
    //     strokeColor(text_color);
    //     moveTo(0, height / 2.f);
    //     lineTo(width, height / 2.f);
    //     stroke();
    //     closePath();
    // }

    // draw waveform
    beginPath();
    lineJoin(BEVEL);
    lineCap(SQUARE);
    strokeColor(text_color);
    strokeWidth(reduced ? 1.5f * scale_factor : 1.f);
    moveTo(0, half);

    for (int i = 0; i < waveformMin.size(); i++)
    {
        if (reduced)
        {
            moveTo((i * level_of_detail) + 1, half - waveformMax[i] * half);
            lineTo((i * level_of_detail) + 1, half - waveformMin[i] * half);
        }
        else
            lineTo((i * level_of_detail) + 1, half - waveformMax[i] * half);
    }
    stroke();
    closePath();

    if (wfFeatures != nullptr)
    {
        Color f_color(feature_color);
        f_color.alpha = 0.8f;
        strokeColor(f_color);
        for (int i = 0; i < wfFeatures->size(); i++)
        {
            WaveformFeature f = wfFeatures->at(i);
            if (f.type == FeatureType::Onset)
            {
                float onsetStart = (float)(f.start - visibleStart) / range;

                if (onsetStart > 1.0f || onsetStart < 0.0f)
                    continue;

                beginPath();
                if (i == featureHighlight)
                    strokeWidth(3.0f);
                else
                    strokeWidth(1.0f);
                moveTo(onsetStart * width + 1, 0);
                lineTo(onsetStart * width + 1, height);
                stroke();
                closePath();
            }
        }
    }

    //  draw selected region
    if (selectable)
    {
        float cursorPosStart = (float)(waveformSelectStart - visibleStart) / range;
        float x1 = width * std::clamp(cursorPosStart, 0.0f, 1.0f);
        beginPath();
        fillColor(cursor_color);
        rect(x1, 0, 2, height);
        fill();
        closePath();
    }

    // draw minimap if zoomed in
    if (visibleStart > 0 || visibleEnd < waveformLength)
    {
        beginPath();
        fillColor(Color(background_color, Color(255, 255, 255), 0.2f));
        rect(0, height - 12.0f, width, 12.0f);
        fill();
        closePath();

        float x;
        if (selectable)
        {
            beginPath();
            fillColor(cursor_color);
            x = (float)waveformSelectStart / waveformLength * width;
            // float w = (float)(waveformSelectEnd - waveformSelectStart) / waveformLength * width;
            rect(x, height - 12.0f, 2, 12.0f);
            fill();
            closePath();
        }

        beginPath();
        strokeWidth(1.0f);
        strokeColor(text_color);
        x = (float)visibleStart / waveformLength * width;
        float w = (float)(visibleEnd - visibleStart) / waveformLength * width;
        rect(x + 1.0f, height - 12.0f, w - 2.0f, 12.0f - 1.0f);
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

            if (featureHighlight >= 0)
                waveformSelectStart = wfFeatures->at(featureHighlight).start;
            else
                waveformSelectStart = cursorPos + visibleStart;

            // if (waveformLength > 0)
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
    // featureHighlight = -1;
    if (dragAction == NONE)
    {
        if (!contains(ev.pos))
            return false;

        int f = getNearestFeature(ev.pos.getX());
        if (featureHighlight != f)
        {
            featureHighlight = f;
            repaint();
        }
        return true;
    }

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

    float dX;
    int dV;

    switch (dragAction)
    {
    case SELECTING:
        break;
    case SCROLLING:
        dX = ev.pos.getX() - clickStart.getX();
        dV = (int)(dX * (waveformLength / getWidth()));

        if (visibleStart + dV <= 0)
            dV = -visibleStart;

        if (visibleEnd + dV >= waveformLength)
            dV = waveformLength - visibleEnd;

        if (dV == 0)
            break;

        visibleEnd += dV;
        visibleStart += dV;

        clickStart = ev.pos;

        calculateWaveform();
        break;
    default:
        break;
    }

    // repaint();
    return true;
}

bool Waveform::onScroll(const ScrollEvent &ev)
{
    if (!contains(ev.pos) || !zoomable || wf == nullptr || wf->size() == 0)
        return false;

    float p = ev.pos.getX() / getWidth();
    int range = visibleEnd - visibleStart;
    int cursorPos = visibleStart + range * p;
    int newRange = range * (1.0f + ev.delta.getY() * 0.05f);
    newRange = std::max(newRange, (int)getWidth());
    int newCursorPos = visibleStart + newRange * p;

    float dX = ev.delta.getX();
    int dV = (int)(dX * (waveformLength / getWidth()));

    if (visibleStart + dV <= 0)
        dV = -visibleStart;

    if (visibleEnd + dV >= waveformLength)
        dV = waveformLength - visibleEnd;

    visibleEnd += dV;
    visibleStart += dV;

    visibleStart -= (newCursorPos - cursorPos);
    visibleEnd = visibleStart + newRange;

    if (visibleEnd >= waveformLength)
    {
        int delta = visibleEnd - waveformLength;
        visibleEnd -= delta;
        visibleStart -= delta;
    }

    if (visibleStart < 0)
        visibleStart = 0;

    calculateWaveform();
    return true;
}

int Waveform::getNearestFeature(float x)
{
    if (wfFeatures == nullptr)
        return -1;

    float width = getWidth();
    float range = visibleEnd - visibleStart;

    int index = -1;

    // find nearest feature and set to highlight
    for (int i = 0; i < wfFeatures->size(); i++)
    {
        float fX = (float)(wfFeatures->at(i).start - visibleStart) / range;
        if (fX < 0.0f || fX >= 1.0f)
            continue;

        if (std::abs(x - fX * width) < 4.0f)
            index = i;
    }

    return index;
}

void Waveform::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO