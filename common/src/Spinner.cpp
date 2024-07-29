#include "Spinner.hpp"

START_NAMESPACE_DISTRHO

Spinner::Spinner(Widget *widget) noexcept
    : WAIVEWidget(widget), loading(false), angle(0.0f), foreground_color(Color(80, 80, 80)) {}

void Spinner::setLoading(bool l)
{
    if (!loading && l)
        angle = 0.0f;

    loading = l;

    if (l)
        getWindow().addIdleCallback(this);
    else
        getWindow().removeIdleCallback(this);

    repaint();
}

void Spinner::idleCallback()
{
    if (!isVisible() || !loading)
        return;
    angle += 0.02f;
    repaint();
}

void Spinner::onNanoDisplay()
{
    if (!loading)
        return;

    const float width = getWidth();
    const float height = getHeight();

    const float center_x = width / 2.0f;
    const float center_y = height / 2.0f;
    const float radius = std::min(center_x, center_y);

    beginPath();
    strokeWidth(4 * scale_factor);
    strokeColor(foreground_color);
    arc(center_x, center_y, radius - 2, angle * M_PI, (angle + 1.5f) * M_PI, NanoVG::CW);
    stroke();
    closePath();
}

END_NAMESPACE_DISTRHO