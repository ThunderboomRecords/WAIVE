#include "TagMap.hpp"

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::Color;

TagMap::TagMap(Widget *widget) noexcept
    : WAIVEWidget(widget),
      c0(Color::fromHSL(0.0f, 0.8f, 0.7f)),
      c1(Color::fromHSL(0.2f, 0.8f, 0.7f)),
      c2(Color::fromHSL(0.6f, 0.8f, 0.7f)),
      c3(Color::fromHSL(0.8f, 0.8f, 0.7f)),
      zoomLevel(1.0f),
      centerPos({0.0, 0.0}),
      highlighted(nullptr),
      tagList(nullptr),
      dragAction(DragAction::NONE),
      callback(nullptr)
{
    // loadSharedResources();
}

// pMap = ((pEmbed - cP) * zL * 0.5 + 0.5) * width
// pEmbed = ((pMap / width) - 0.5) / (0.5 * zL) + cP

Point<double> TagMap::embeddingToMap(Point<double> p)
{
    const float width = getWidth();
    const float height = getHeight();

    Point<double> pMap{
        ((p.getX() - centerPos.getX()) * zoomLevel * 0.5 + 0.5) * width,
        ((p.getY() - centerPos.getY()) * zoomLevel * 0.5 + 0.5) * height,
    };

    return pMap;
}

Point<double> TagMap::mapToEmbedding(Point<double> p)
{
    const float width = getWidth();
    const float height = getHeight();

    Point<double> pEmbed{
        ((p.getX() / width) - 0.5) / (0.5 * zoomLevel) + centerPos.getX(),
        ((p.getY() / height) - 0.5) / (0.5 * zoomLevel) + centerPos.getY(),
    };

    return pEmbed;
}

bool TagMap::onMouse(const MouseEvent &ev)
{
    if (!isVisible())
        return false;

    if (ev.press && contains(ev.pos))
    {
        if (ev.button == MouseButton::kMouseButtonLeft)
            dragAction = CLICKING;

        return false;
    }
    else if (!ev.press && ev.button == MouseButton::kMouseButtonLeft && dragAction != NONE)
    {
        switch (dragAction)
        {
        case CLICKING:
            if (highlighted == nullptr)
                return false;

            if (selected.count(highlighted->id))
                selected.erase(highlighted->id);
            else
                selected.insert({highlighted->id, highlighted});

            if (callback != nullptr)
                callback->tagMapUpdated(this);

            repaint();
            break;
        case SCROLLING:
            break;
        default:
            break;
        }

        dragAction = NONE;
    }

    return false;
};

bool TagMap::onMotion(const MotionEvent &ev)
{
    if (!isVisible())
        return false;

    if (dragAction == NONE)
    {
        if (!contains(ev.pos))
            return false;

        // find nearest in Map space, to be be consistent with
        // distance from mouse

        Tag *nearest = nullptr;
        float d = INFINITY;

        for (int i = 0; i < tagList->size(); i++)
        {
            Point<double> sPos = embeddingToMap({
                tagList->at(i).embedX,
                tagList->at(i).embedY,
            });

            float dX = sPos.getX() - ev.pos.getX();
            float dY = sPos.getY() - ev.pos.getY();
            float dS = dX * dX + dY * dY;
            if (dS < 100.0f && dS < d)
            {
                nearest = &tagList->at(i);
                d = dS;
            }
        }

        if (highlighted != nearest)
        {
            highlighted = nearest;
            repaint();
        }

        return false;
    }
    else if (dragAction == CLICKING)
    {
        dragAction = SCROLLING;
        dragStart = Point<double>(ev.pos);
        centerStart = Point<double>{centerPos.getX(), centerPos.getY()};
        return false;
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

bool TagMap::onScroll(const ScrollEvent &ev)
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

Color TagMap::get2DColor(float x, float y) const
{
    // expects x, y in [-1, 1]
    Color cx1 = Color(c0, c1, x * 0.5f + 0.5f);
    Color cx2 = Color(c3, c2, x * 0.5f + 0.5f);
    return Color(cx1, cx2, y * 0.5f + 0.5f);
}

void TagMap::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(background_color);
    rect(0, 0, width, height);
    fill();
    closePath();

    Color hC;
    float hX, hY;

    for (const auto &element : *tagList)
    {
        float embedX = element.embedX;
        float embedY = element.embedY;

        Point<double> pMap = embeddingToMap({embedX, embedY});

        if (!contains(pMap))
            continue;

        float r = 2.0f * scale_factor;
        if (highlighted == &element)
            r *= 2.0f;

        beginPath();
        if (selected.count(element.id))
            fillColor(get2DColor(embedX, embedY));
        else
            fillColor(highlight_color);
        circle(pMap.getX(), pMap.getY(), r);
        fill();
        closePath();

        if (highlighted == &element)
        {
            hC = get2DColor(embedX, embedY);
            hX = pMap.getX();
            hY = pMap.getY();
        }
    }

    if (highlighted != nullptr && highlighted->name.size() != 0)
    {
        DGL::Rectangle<float> bounds;
        fontSize(getFontSize());
        fontFaceId(font);
        textBounds(0, 0, highlighted->name.c_str(), nullptr, bounds);

        beginPath();
        fillColor(hC);
        rect(hX, hY - bounds.getHeight() - 4, bounds.getWidth() + 4, bounds.getHeight() + 4);
        fill();
        closePath();

        beginPath();
        fillColor(text_color);
        textAlign(Align::ALIGN_LEFT | Align::ALIGN_BOTTOM);
        text(hX + 2, hY - 2, highlighted->name.c_str(), nullptr);
        closePath();
    }
}

void TagMap::setSelectAll(bool all)
{
    if (all)
    {
        for (auto &element : *tagList)
            selected.insert({element.id, &element});
    }
    else
        selected.clear();

    if (callback != nullptr)
        callback->tagMapUpdated(this);

    repaint();
}

std::string TagMap::getSelectedTagList()
{
    std::string res = "";
    for (const auto &element : selected)
    {
        if (res.length() > 0)
            res += ", ";
        res += std::to_string(element.first);
    }

    std::cout << res << std::endl;

    return res;
}

void TagMap::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO