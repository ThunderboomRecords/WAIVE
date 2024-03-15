#include "ScoreGrid.hpp"

START_NAMESPACE_DISTRHO


ScoreGrid::ScoreGrid(Widget *parent) noexcept
    : NanoSubWidget(parent),
      selected_16th(-1),
      selected_ins(-1)
{

}

bool ScoreGrid::onMouse(const MouseEvent &ev){
    if(!contains(ev.pos) || !ev.press) return false;

    return true;
}

bool ScoreGrid::onMotion(const MotionEvent &ev){ 
    Window &window = getWindow();

    if(contains(ev.pos)){
        window.setCursor(kMouseCursorHand);

        const float width = getWidth();
        const float height = getHeight();

        int i = (int) std::floor(16.0f * ev.pos.getX() / width);
        int j = 8 - (int) std::floor(9.0f * ev.pos.getY() / height);

        i = std::min(std::max(i, 0), 15);
        j = std::min(std::max(j, 0), 8);

        if(selected_16th != i || selected_ins != j){
            selected_16th = i;
            selected_ins = j;

            repaint();
        }

        return true;
    } else {
        window.setCursor(kMouseCursorArrow);

        if(selected_16th != -1 || selected_ins != -1){
            selected_16th = -1;
            selected_ins = -1;

            repaint();
        }
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

    if(selected_16th >= 0 && selected_ins >= 0){
        float x = selected_16th*gridWidth;
        float y = (8 - selected_ins)*gridHeight;

        beginPath();
        strokeColor(255, 255, 255);
        strokeWidth(3);
        rect(x, y, gridWidth, gridHeight);
        stroke();
        closePath();
    }

}

END_NAMESPACE_DISTRHO