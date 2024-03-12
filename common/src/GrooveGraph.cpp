#include "GrooveGraph.hpp"

START_NAMESPACE_DISTRHO


GrooveGraph::GrooveGraph(Widget *parent) noexcept
    : NanoSubWidget(parent)
{
    
}

bool GrooveGraph::onMouse(const MouseEvent &ev){ return false; }
bool GrooveGraph::onMotion(const MotionEvent &ev){ return false; }

void GrooveGraph::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    const float gridWidth = width/16.0f;

    printf("GrooveGraph::onNanoDisplay()\n");

    beginPath();
    fillColor(Color(40, 40, 40));
    rect(0, 0, width, height);
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

    // auto groove_a = groove->accessor<float, 2>();

    for(int i = 0; i < 16; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            int idx = i*3 + j;
            if((*fGroove)[idx][0] < 0.3f) {
                break;
            }

            // printf("%d %.2f\n", idx, (*fGroove)[idx][0]);

            float velocity = (*fGroove)[idx][1];
            float offset = (*fGroove)[idx][2];

            float s = (velocity + 1) * height;

            float x = (i + offset)*gridWidth;

            beginPath();
            strokeColor(0, 200, 50);
            strokeWidth(3.0f);
            moveTo(x, (height - s)/2);
            lineTo(x, (height + s)/2);
            stroke();
            closePath();
        }

    }

}

END_NAMESPACE_DISTRHO