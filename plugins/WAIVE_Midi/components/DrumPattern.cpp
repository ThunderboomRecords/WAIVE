#include "DrumPattern.hpp"

START_NAMESPACE_DISTRHO


DrumPattern::DrumPattern(Widget *parent) noexcept
    : NanoSubWidget(parent)
{
    for(int i = 0; i < 9; i++)
    {
        midiToRow.insert({midiMap[i], i});
    }

}

bool DrumPattern::onMouse(const MouseEvent &ev){ return false; }
bool DrumPattern::onMotion(const MotionEvent &ev){ return false; }

void DrumPattern::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    const float gridWidth = width/16.0f;
    const float gridHeight = height/9.0f;

    beginPath();
    fillColor(Color(40, 40, 40));
    rect(0, 0, width, height);
    fill();
    closePath();

    beginPath();
    fillColor(60, 60, 60);
    rect(4*gridWidth, 0, 4*gridWidth, height);
    fill();
    closePath();

    beginPath();
    fillColor(60, 60, 60);
    rect(12*gridWidth, 0, 4*gridWidth, height);
    fill();
    closePath();

    for(int i = 1; i < 16; i++)
    {
        beginPath();
        strokeColor(80, 80, 80);
        moveTo(i*gridWidth, 0);
        lineTo(i*gridWidth, height);
        stroke();
        closePath();
    }

    for(int i = 1; i < 9; i++)
    {
        beginPath();
        strokeColor(80, 80, 80);
        moveTo(0, i*gridHeight);
        lineTo(width, i*gridHeight);
        stroke();
        closePath();
    }

    std::vector<Note>::iterator noteStart = notes->begin();
    std::vector<Note>::iterator noteEnd = notes->begin();
    float tpb = 1920.0f;
    float beatWidth = width / 4.0f;

    for(; noteStart != notes->end(); ++noteStart)
    {
        if((*noteStart).noteOn)
        {
            uint8_t currentNote = (*noteStart).midiNote;
            noteEnd = noteStart;

            for(; noteEnd != notes->end(); ++noteEnd)
            {
                if(!(*noteEnd).noteOn && (*noteEnd).midiNote == currentNote)
                {
                    // found noteOff, render rectangle
                    int startTick = (*noteStart).tick;
                    int endTick =  (*noteEnd).tick;
                    int row = midiToRow[currentNote];

                    float x = beatWidth * (startTick / tpb);
                    float y = gridHeight * (8 - row);
                    float h = gridHeight;
                    float w = beatWidth * (endTick - startTick) / tpb;

                    int velocity = (*noteStart).velocity;

                    float hue = (8 - row)/10.0f;
                    Color base = Color::fromHSL(hue, 0.8f, 0.1f);
                    Color top = Color::fromHSL(hue, 0.8f, 0.8f);
                    base.interpolate(top, velocity/255.0f);

                    beginPath();
                    strokeColor(40, 40, 40);
                    fillColor(base);
                    rect(x, y, w, h);
                    fill();
                    stroke();
                    closePath();

                    break;
                }
            }
        }
    }

    // round off corners
    float r = 8.0f;
    fillColor(240, 240, 240);
    strokeColor(255, 0, 0);

    // top left
    beginPath();
    moveTo(-1, -1);
    lineTo(-1, r);
    arcTo(-1, -1, r, -1, r);
    closePath();
    fill();

    // top right
    beginPath();
    moveTo(width+1, -1);
    lineTo(width+1, r);
    arcTo(width+1, -1, width - r, -1, r);
    closePath();
    fill();

    // bottom left
    beginPath();
    moveTo(-1, height+1);
    lineTo(-1, height-r);
    arcTo(-1, height+1, r, height+1, r);
    closePath();
    fill();

    // bottom right
    beginPath();
    moveTo(width+1, height+1);
    lineTo(width+1, height-r);
    arcTo(width+1, height+1, width-r, height+1, r);
    closePath();
    fill();
}


END_NAMESPACE_DISTRHO