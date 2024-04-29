#include "Waveform.hpp"

START_NAMESPACE_DISTRHO

Waveform::Waveform(Widget *widget) noexcept
    : NanoSubWidget(widget),
      backgroundColor(Color(40, 40, 40)),
      lineColor(Color(200, 200, 200)),
      waveformCached(false)
{

}

void Waveform::calculateWaveform(std::vector<float> *wf)
{
    waveformCached = false;

    int size = wf->size();
    if(size == 0) return;

    const int width = getWidth();
    double samples_per_pixel = size / (double) (width);

    waveformMin.resize(width);
    waveformMax.resize(width);

    float fIndex = 0.0f;
    int index = fIndex;


    index = 0;
    for(int i = 0; i < width; i++)
    {
        waveformMin[i] = 0.0f;
        waveformMax[i] = 0.0f;
        for(int j = 0; j < samples_per_pixel; j++)
        {
            try
            {
                waveformMax[i] = std::max(waveformMax[i], wf->at(index));
                waveformMin[i] = std::min(waveformMin[i], wf->at(index));
                index++;
            }
            catch(const std::out_of_range& e)
            {
                break;
            }
        }

        waveformMax[i] = std::clamp(waveformMax[i], -1.0f, 1.0f);
        waveformMin[i] = std::clamp(waveformMin[i], -1.0f, 1.0f);
    }

    waveformCached = true;
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

    if(!waveformCached) return;

    beginPath();
    strokeColor(lineColor);
    strokeWidth(1.0f);
    moveTo(0, half);

    for(int i = 0; i < waveformMin.size(); i++)
    {
        float min = waveformMin[i];
        float max = waveformMax[i];

        moveTo(i, half - max * half);
        lineTo(i, half - min * half);
    }
    stroke();
    closePath();
}


END_NAMESPACE_DISTRHO