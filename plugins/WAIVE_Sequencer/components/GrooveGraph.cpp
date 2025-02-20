#include "GrooveGraph.hpp"

START_NAMESPACE_DISTRHO

GrooveGraph::GrooveGraph(Widget *parent) noexcept
    : WAIVEWidget(parent),
      dragging(false),
      callback(nullptr),
      fGroove(nullptr)
{
}

bool GrooveGraph::onMouse(const MouseEvent &ev)
{
    if (!contains(ev.pos) && !dragging)
        return false;

    if (ev.press)
    {
        dragging = true;
        dragStart = ev.pos;
    }
    else
    {
        dragging = false;
    }

    return false;
}

bool GrooveGraph::onMotion(const MotionEvent &ev)
{
    Window &window = getWindow();

    if (contains(ev.pos))
    {
        window.setCursor(kMouseCursorHand);

        if (dragging)
        {
            // get nearest point
        }

        return true;
    }

    return false;
}

bool GrooveGraph::onScroll(const ScrollEvent &ev)
{
    if (!contains(ev.pos))
        return false;

    float position = ev.pos.getX() / getWidth();
    int nearestIndex = nearestElementAtPos(position);
    if (nearestIndex < 0)
        return true;

    float velocity = fGroove->at(nearestIndex).velocity;
    float newVelocity = velocity + ev.delta.getY() * 0.05;
    newVelocity = std::clamp(newVelocity, 0.0f, 1.0f);

    if (newVelocity == velocity)
        return true;

    fGroove->at(nearestIndex).velocity = newVelocity;

    if (callback != nullptr)
        callback->grooveClicked(this);

    return true;
}

int GrooveGraph::nearestElementAtPos(float position)
{
    int index = -1;
    float bestDist = std::numeric_limits<float>::infinity();

    std::vector<GrooveEvent>::iterator grooveEvents = fGroove->begin();
    for (; grooveEvents != fGroove->end(); grooveEvents++)
    {
        float dist = std::abs(position - (*grooveEvents).position);
        if (dist < bestDist)
        {
            bestDist = dist;
            index = grooveEvents - fGroove->begin();
        }
    }

    return index;
}

void GrooveGraph::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    const float gridWidth = width / 16.0f;

    beginPath();
    fillColor(WaiveColors::grey2);
    rect(0, 0, width, height);
    closePath();
    fill();

    beginPath();
    fillColor(WaiveColors::grey3);
    rect(4 * gridWidth, 0, 4 * gridWidth, height);
    rect(12 * gridWidth, 0, 4 * gridWidth, height);
    fill();
    closePath();

    beginPath();
    strokeColor(WaiveColors::grey1);
    for (int i = 1; i < 16; i++)
    {
        moveTo(i * gridWidth, 0);
        lineTo(i * gridWidth, height);
    }
    stroke();
    closePath();

    std::vector<GrooveEvent>::iterator grooveEvents = fGroove->begin();

    for (; grooveEvents != fGroove->end(); grooveEvents++)
    {
        float velocity = (*grooveEvents).velocity;
        float position = (*grooveEvents).position;

        float x = position * width;
        beginPath();
        strokeColor(Color(88, 88, 207, velocity));
        strokeWidth(3.0f);
        moveTo(x, (height - velocity * height) / 2.0f);
        lineTo(x, (height + velocity * height) / 2.0f);
        closePath();
        stroke();
    }

    // // round off corners
    // float r = 8.0f;
    // fillColor(WaiveColors::grey1);
    // strokeColor(255, 0, 0);

    // // top left
    // beginPath();
    // moveTo(-1, -1);
    // lineTo(-1, r);
    // arcTo(-1, -1, r, -1, r);
    // closePath();
    // fill();

    // // top right
    // beginPath();
    // moveTo(width + 1, -1);
    // lineTo(width + 1, r);
    // arcTo(width + 1, -1, width - r, -1, r);
    // closePath();
    // fill();

    // // bottom left
    // beginPath();
    // moveTo(-1, height + 1);
    // lineTo(-1, height - r);
    // arcTo(-1, height + 1, r, height + 1, r);
    // closePath();
    // fill();

    // // bottom right
    // beginPath();
    // moveTo(width + 1, height + 1);
    // lineTo(width + 1, height - r);
    // arcTo(width + 1, height + 1, width - r, height + 1, r);
    // closePath();
    // fill();
}

END_NAMESPACE_DISTRHO