#include "DrumPattern.hpp"

START_NAMESPACE_DISTRHO


DrumPattern::DrumPattern(Widget *parent) noexcept
    : NanoSubWidget(parent)
{
    s_map[0] = 0;
    for(int i=1; i<9; i++)
    {
        s_map[i] = s_map[i-1] + max_events[i-1];
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

    for(int i = 0; i < 16; i++)
    {
        for(int j = 0; j < 9; j++)
        {
            for(int k = 0; k < max_events[j]; k++)
            {
                int index = s_map[j] + k;
                if((*fDrumPattern)[i][index][0] < 0.3f) {
                    continue;
                }

                float velocity = (*fDrumPattern)[i][index][1];
                velocity = 0.5f * (velocity + 1.0f);
                float offset = (*fDrumPattern)[i][index][2];

                float x = (i+offset)*gridWidth;
                float y = (8 - j)*gridHeight;

                beginPath();
                strokeColor(200, 200, 200);
                fillColor(0, 200, 50, (int)(velocity*255.0f));
                rect(x, y, gridWidth, gridHeight);
                fill();
                stroke();
                closePath();
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