#include "ScoreGrid.hpp"

START_NAMESPACE_DISTRHO

ScoreGrid::ScoreGrid(Widget *parent) noexcept
    : WAIVEWidget(parent)
{
    ui = nullptr;
    fScore = nullptr;
    selected_16th = -1;
    selected_ins = -1;
}

bool ScoreGrid::onMouse(const MouseEvent &ev)
{
    if (!contains(ev.pos) || !ev.press || ev.button != kMouseButtonLeft || fScore == nullptr)
        return false;

    if (selected_16th != -1 && selected_ins != -1)
    {

        if ((*fScore)[selected_16th][selected_ins] < 0.5)
        {
            (*fScore)[selected_16th][selected_ins] = 1.0f;
        }
        else
        {
            (*fScore)[selected_16th][selected_ins] = 0.0f;
        }

        repaint();

        if (ui != nullptr)
            ui->setState("score", "new");
    }

    return true;
}

bool ScoreGrid::onMotion(const MotionEvent &ev)
{
    Window &window = getWindow();

    if (contains(ev.pos))
    {
        window.setCursor(kMouseCursorHand);

        const float width = getWidth();
        const float height = getHeight();

        int i = (int)std::floor(16.0f * ev.pos.getX() / width);
        int j = 8 - (int)std::floor(9.0f * ev.pos.getY() / height);

        i = std::min(std::max(i, 0), 15);
        j = std::min(std::max(j, 0), 8);

        if (selected_16th != i || selected_ins != j)
        {
            selected_16th = i;
            selected_ins = j;

            repaint();
        }

        return true;
    }
    else
    {
        window.setCursor(kMouseCursorArrow);

        if (selected_16th != -1 || selected_ins != -1)
        {
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

    const float gridWidth = width / 16.0f;
    const float gridHeight = height / 9.0f;

    beginPath();
    fillColor(WaiveColors::grey2);
    rect(0, 0, width, height);
    fill();
    closePath();

    beginPath();
    fillColor(WaiveColors::light1);
    rect(4 * gridWidth, 0, 4 * gridWidth, height);
    rect(12 * gridWidth, 0, 4 * gridWidth, height);
    fill();
    closePath();

    beginPath();
    strokeColor(WaiveColors::light2);
    for (int i = 1; i < 16; i++)
    {
        moveTo(i * gridWidth, 0);
        lineTo(i * gridWidth, height);
    }

    for (int i = 1; i < 9; i++)
    {
        moveTo(0, i * gridHeight);
        lineTo(width, i * gridHeight);
    }
    stroke();
    closePath();

    if (fScore == nullptr)
        return;

    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            if ((*fScore)[i][j] < 0.5f)
                continue;

            float x = i * gridWidth;
            float y = (8 - j) * gridHeight;

            beginPath();
            strokeColor(foreground_color);

            float hue = (8 - j) / 10.0f;

            fillColor(Color::fromHSL(hue, 0.8f, 0.7f));
            rect(x, y, gridWidth, gridHeight);
            fill();
            stroke();
            closePath();
        }
    }

    // round off corners
    float r = 8.0f;
    fillColor(WaiveColors::grey1);
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
    moveTo(width + 1, -1);
    lineTo(width + 1, r);
    arcTo(width + 1, -1, width - r, -1, r);
    closePath();
    fill();

    // bottom left
    beginPath();
    moveTo(-1, height + 1);
    lineTo(-1, height - r);
    arcTo(-1, height + 1, r, height + 1, r);
    closePath();
    fill();

    // bottom right
    beginPath();
    moveTo(width + 1, height + 1);
    lineTo(width + 1, height - r);
    arcTo(width + 1, height + 1, width - r, height + 1, r);
    closePath();
    fill();

    if (selected_16th >= 0 && selected_ins >= 0)
    {
        float x = selected_16th * gridWidth;
        float y = (8 - selected_ins) * gridHeight;

        beginPath();
        strokeColor(accent_color);
        strokeWidth(3);
        rect(x, y, gridWidth, gridHeight);
        stroke();
        closePath();
    }
}

END_NAMESPACE_DISTRHO