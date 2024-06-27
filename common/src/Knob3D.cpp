#include "Knob3D.hpp"

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::Color;

Knob3D::Knob3D(Widget *parent) noexcept
    : Knob(parent),
      line_color(Color(100, 100, 100)),
      knob_color(Color(180, 180, 180))
{
}

void Knob3D::setKnobColor(Color color)
{
    const float width = getWidth();
    const float height = getHeight();

    knob_color = color;
}

void Knob3D::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    if (renderDebug)
    {
        beginPath();
        strokeColor(accent_color);
        rect(0, 0, width, height);
        stroke();
        closePath();
    }

    const float center_x = width / 2.0f;
    const float center_y = height / 2.0f;

    float normValue = (getValue() - min) / (max - min);
    if (normValue < 0.0f)
        normValue = 0.0f;

    // draw indicator
    drawIndicator();

    // draw knob
    Color highlight = Color(Color(255, 255, 255), knob_color, 0.05);

    outer_gradient = linearGradient(
        0, 0,
        width, height,
        highlight,
        knob_color);

    inner_gradient = linearGradient(
        width, height,
        0, 0,
        highlight,
        knob_color);

    float rOut = radius - gauge_width - 2.0f;
    float rIn = 0.7f * rOut;

    beginPath();
    circle(center_x, center_y, rOut);
    fillPaint(outer_gradient);
    fill();
    closePath();

    beginPath();
    circle(center_x, center_y, rIn);
    fillPaint(inner_gradient);
    fill();
    closePath();

    // draw knob line
    float angle = (0.75f + 1.5f * normValue) * M_PI;
    float x1 = cos(-angle) * rIn + center_x;
    float y1 = -sin(-angle) * rIn + center_y;
    float x2 = cos(-angle) * radius + center_x;
    float y2 = -sin(-angle) * radius + center_y;

    beginPath();
    strokeColor(accent_color);
    strokeWidth(4.0f);
    moveTo(x1, y1);
    lineTo(x2, y2);
    stroke();
    closePath();

    drawLabel();
}

END_NAMESPACE_DISTRHO