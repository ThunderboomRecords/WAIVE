#include "GrooveGraph.hpp"

START_NAMESPACE_DISTRHO


GrooveGraph::GrooveGraph(Widget *parent) noexcept
    : NanoSubWidget(parent)
{
    
}

bool GrooveGraph::onMouse(const MouseEvent &ev){ 

    if(!ev.press) { return false; }
    if(!contains(ev.pos)) { return false; }

    if(callback != nullptr)
    {
        callback->grooveClicked(this);
        return true;
    }

    return false; 
}
bool GrooveGraph::onMotion(const MotionEvent &ev){ 

    Window &window = getWindow();

    if(contains(ev.pos)){
        window.setCursor(kMouseCursorHand);

        return true;
    } else {
        // window.setCursor(kMouseCursorArrow);
    }
    
    return false; 
}

void GrooveGraph::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    const float gridWidth = width/16.0f;

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