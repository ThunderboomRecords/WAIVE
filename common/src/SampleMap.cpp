#include "SampleMap.hpp"

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::Color;

SampleMap::SampleMap(Widget *widget) noexcept
    : NanoBaseWidget(widget), background_color(Color(100, 100, 100)),
      c0(Color::fromHSL(0.0f, 0.8f, 0.7f)),
      c1(Color::fromHSL(0.2f, 0.8f, 0.7f)),
      c2(Color::fromHSL(0.6f, 0.8f, 0.7f)),
      c3(Color::fromHSL(0.8f, 0.8f, 0.7f)),
      zoomLevel(1.0f),
      centerPos({0.0, 0.0}),
      dragging(false),
      selectedSample(nullptr),
      callback(nullptr)
{
}

// pMap = ((pEmbed - cP) * zL * 0.5 + 0.5) * width
// pEmbed = ((pMap / width) - 0.5) / (0.5 * zL) + cP

Point<double> SampleMap::embeddingToMap(Point<double> p)
{
    const float width = getWidth();
    const float height = getHeight();

    Point<double> pMap{
        ((p.getX() - centerPos.getX()) * zoomLevel * 0.5 + 0.5) * width,
        ((p.getY() - centerPos.getY()) * zoomLevel * 0.5 + 0.5) * height,
    };

    return pMap;
}

Point<double> SampleMap::mapToEmbedding(Point<double> p)
{
    const float width = getWidth();
    const float height = getHeight();

    Point<double> pEmbed{
        ((p.getX() / width) - 0.5) / (0.5 * zoomLevel) + centerPos.getX(),
        ((p.getY() / height) - 0.5) / (0.5 * zoomLevel) + centerPos.getY(),
    };

    return pEmbed;
}

bool SampleMap::onMouse(const MouseEvent &ev)
{
    if (!contains(ev.pos) && !dragging)
        return false;

    if (ev.press)
    {
        if (selectedSample != nullptr && callback != nullptr)
        {
            callback->mapSampleSelected(selectedSample->getId());
        }
        else if (!dragging)
        {
            dragging = true;
            dragStart = Point<double>(ev.pos);
            centerStart = Point<double>{centerPos.getX(), centerPos.getY()};
        }
    }
    else if (!ev.press && dragging)
    {
        dragging = false;
    }
    else
    {
        return false;
    }

    return true;
};

bool SampleMap::onMotion(const MotionEvent &ev)
{
    if (!dragging)
    {
        if (!contains(ev.pos))
        {
            selectedSample = nullptr;
            return false;
        }

        // find nearest in Map space, to be be consistent with
        // distance from mouse

        SampleInfo *nearest = nullptr;
        float d = INFINITY;

        for (int i = 0; i < allSamples->size(); i++)
        {
            Point<double> sPos = embeddingToMap({
                allSamples->at(i).embedX,
                allSamples->at(i).embedY,
            });

            float dX = sPos.getX() - ev.pos.getX();
            float dY = sPos.getY() - ev.pos.getY();
            float dS = dX * dX + dY * dY;
            if (dS < 100.0f && dS < d)
            {
                nearest = &allSamples->at(i);
                d = dS;
            }

            selectedSample = nearest;
        }

        repaint();

        return true;
    }

    const float width = getWidth();
    const float height = getHeight();

    // get mouse delta
    float dx = ev.pos.getX() - dragStart.getX();
    float dy = ev.pos.getY() - dragStart.getY();

    // scale to [-1, 1] coordinate space
    dx = (dx / width) * 2.0f;
    dy = (dy / height) * 2.0f;

    // scale by zoom level
    dx /= zoomLevel;
    dy /= zoomLevel;

    float newX = std::clamp(centerStart.getX() - dx, -0.8, 0.8);
    float newY = std::clamp(centerStart.getY() - dy, -0.8, 0.8);

    centerPos.setX(newX);
    centerPos.setY(newY);

    repaint();
    return true;
};

bool SampleMap::onScroll(const ScrollEvent &ev)
{
    if (!contains(ev.pos))
        return false;

    Point<double> cursorPos1 = mapToEmbedding(ev.pos);

    zoomLevel += ev.delta.getY() / 20.0f;
    zoomLevel = std::clamp(zoomLevel, 0.75f, 4.0f);

    Point<double> deltaTranslate = mapToEmbedding(ev.pos) - cursorPos1;

    centerPos -= deltaTranslate;

    repaint();
    return true;
};

Color SampleMap::get2DColor(float x, float y)
{
    // expects x, y in [-1, 1]
    Color cx1 = Color(c0, c1, x * 0.5f + 0.5f);
    Color cx2 = Color(c3, c2, x * 0.5f + 0.5f);
    return Color(cx1, cx2, y * 0.5f + 0.5f);
}

void SampleMap::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(background_color);
    rect(0, 0, width, height);
    fill();
    closePath();

    for (int i = 0; i < allSamples->size(); i++)
    {
        float embedX = allSamples->at(i).embedX;
        float embedY = allSamples->at(i).embedY;

        Point<double> pMap = embeddingToMap({embedX, embedY});

        if (!contains(pMap))
            continue;

        float r = 3.0f;
        if (selectedSample != nullptr && selectedSample->getId() == allSamples->at(i).getId())
        {
            r = 6.0f;
        }

        beginPath();
        fillColor(get2DColor(embedX, embedY));
        circle(pMap.getX(), pMap.getY(), r);
        fill();
        closePath();
    }
}

void SampleMap::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO