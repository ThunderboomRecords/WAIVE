#include "SampleMap.hpp"

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::Color;

SampleMap::SampleMap(Widget *widget, DragDropManager *manager) noexcept
    : WAIVEWidget(widget),
      DragDropWidget(manager),
      c0(Color::fromHSL(0.0f, 0.8f, 0.7f)),
      c1(Color::fromHSL(0.2f, 0.8f, 0.7f)),
      c2(Color::fromHSL(0.6f, 0.8f, 0.7f)),
      c3(Color::fromHSL(0.8f, 0.8f, 0.7f)),
      zoomLevel(1.0f),
      centerPos({0.0, 0.0}),
      dragAction(DragAction::NONE),
      highlightSample(-1),
      contextMenuSample(-1),
      selectedSample(nullptr),
      preview(true),
      callback(nullptr)
{
    menu = new Menu(widget);
    for (int i = 1; i < NUM_SLOTS; i++)
        menu->addItem(fmt::format("Add to slot {:d}", i));
    menu->setFont("Poppins Medium", Poppins_Medium, Poppins_Medium_len);
    menu->setFontSize(14.f);
    menu->calculateWidth();
    menu->setDisplayNumber(8);
    menu->hide();
    menu->setCallback(this);
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
    if (!isVisible())
        return false;

    if (ev.press && contains(ev.pos))
    {
        if (ev.button == MouseButton::kMouseButtonLeft)
            dragAction = CLICKING;
        else if (ev.button == MouseButton::kMouseButtonRight)
        {
            if (highlightSample >= 0)
            {
                contextMenuSample = highlightSample;
                menu->setAbsolutePos(
                    ev.pos.getX() + getAbsoluteX() - 2,
                    ev.pos.getY() + getAbsoluteY() - 2);
                menu->toFront();
                menu->show();
            }
        }
        return false;
    }
    else if (!ev.press && ev.button == MouseButton::kMouseButtonLeft && dragAction != NONE)
    {
        switch (dragAction)
        {
        case CLICKING:
            if (callback != nullptr)
                callback->mapSampleSelected(highlightSample);
            break;
        case SCROLLING:
            break;
        default:
            break;
        }

        dragAction = NONE;
    }
    else
    {
        return false;
    }

    return false;
};

bool SampleMap::onMotion(const MotionEvent &ev)
{
    if (!isVisible())
        return false;

    if (dragAction == NONE)
    {
        if (!contains(ev.pos))
        {
            highlightSample = -1;

            return false;
        }

        // find nearest in Map space, to be be consistent with
        // distance from mouse

        int nearest = -1;
        float d = INFINITY;

        for (int i = 0; i < allSamples->size(); i++)
        {
            Point<double> sPos = embeddingToMap({
                allSamples->at(i)->embedX,
                allSamples->at(i)->embedY,
            });

            float dX = sPos.getX() - ev.pos.getX();
            float dY = sPos.getY() - ev.pos.getY();
            float dS = dX * dX + dY * dY;
            if (dS < 100.0f && dS < d)
            {
                nearest = allSamples->at(i)->getId();
                d = dS;
            }
        }

        if (highlightSample != nearest)
        {
            highlightSample = nearest;

            if (callback != nullptr && preview)
                callback->mapSampleHovered(highlightSample);

            repaint();
        }

        return false;
    }
    else if (dragAction == CLICKING)
    {

        if (highlightSample >= 0)
        {
            dragAction = DragAction::NONE;
            dragDropManager->dragDropStart(this, fmt::format("{:d}", highlightSample));
        }
        else
        {
            dragAction = SCROLLING;
            dragStart = Point<double>(ev.pos);
            centerStart = Point<double>{centerPos.getX(), centerPos.getY()};
        }

        return true;
    }
    else if (dragAction == SCROLLING)
    {
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
        return false;
    }

    return false;
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

void SampleMap::onMenuItemSelection(Menu *menu, int item, const std::string &value)
{
    // std::cout << "put sample " << contextMenuSample << " into slot " << item << std::endl;
    if (callback != nullptr)
        callback->mapSampleLoadSlot(contextMenuSample, item);
}

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

    float hX, hY;
    Color hC;
    int hI = -1;

    for (int i = 0; i < allSamples->size(); i++)
    {
        float embedX = allSamples->at(i)->embedX;
        float embedY = allSamples->at(i)->embedY;

        Point<double> pMap = embeddingToMap({embedX, embedY});

        if (!contains(pMap))
            continue;

        int sampleId = allSamples->at(i)->getId();

        float r = 4.0f * scale_factor;
        if (highlightSample > -1 && highlightSample == sampleId)
        {
            r *= 2.0f;
            hC = get2DColor(embedX, embedY);
            hX = pMap.getX();
            hY = pMap.getY();
            hI = i;
        }

        beginPath();
        fillColor(get2DColor(embedX, embedY));
        circle(pMap.getX(), pMap.getY(), r);
        fill();
        closePath();

        if (selectedSample != nullptr && selectedSample->get() != nullptr && selectedSample->get()->getId() == sampleId)
        {
            beginPath();
            strokeWidth(2.0f);
            strokeColor(255, 255, 255);
            circle(pMap.getX(), pMap.getY(), r);
            stroke();
            closePath();
        }
    }

    if (hI > -1)
    {
        // Draw sample name
        std::string name = allSamples->at(hI)->name;
        Rectangle<float> bounds;
        fontSize(getFontSize());
        fontFaceId(font);
        textBounds(0, 0, name.c_str(), nullptr, bounds);

        beginPath();
        fillColor(hC);
        rect(hX, hY - bounds.getHeight() - 4, bounds.getWidth() + 4, bounds.getHeight() + 4);
        fill();
        closePath();

        beginPath();
        fillColor(text_color);
        textAlign(Align::ALIGN_LEFT | Align::ALIGN_BOTTOM);
        text(hX + 2, hY - 2, name.c_str(), nullptr);
        closePath();
    }
}

void SampleMap::buttonClicked(Button *btn)
{
    std::cout << "SampleMap::buttonClicked" << std::endl;
    if (callback != nullptr)
        callback->mapSampleImport();
}

void SampleMap::dataAccepted(DragDropWidget *destination)
{
    highlightSample = -1;
}

void SampleMap::dataRejected(DragDropWidget *destination)
{
    highlightSample = -1;
}

void SampleMap::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO