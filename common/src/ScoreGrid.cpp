#include "ScoreGrid.hpp"

START_NAMESPACE_DISTRHO


ScoreGrid::ScoreGrid(Widget *parent) noexcept
    : NanoSubWidget(parent)
{

}

bool ScoreGrid::onMouse(const MouseEvent &ev){ return false; }
bool ScoreGrid::onMotion(const MotionEvent &ev){ 
    Window &window = getWindow();
    if(contains(ev.pos)){
        window.setCursor(kMouseCursorHand);
        return true;
    } else {
        window.setCursor(kMouseCursorArrow);
    }
    return false; 
}

void ScoreGrid::onNanoDisplay()
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
            if((*fScore)[i][j] < 0.5f) {
                continue;
            }

            float x = i*gridWidth;
            float y = (8 - j)*gridHeight;

            beginPath();
            strokeColor(200, 200, 200);
            fillColor(0, 200, 50);
            rect(x, y, gridWidth, gridHeight);
            fill();
            stroke();
            closePath();
        }
    }

}

END_NAMESPACE_DISTRHO