#include "SourceList.hpp"

START_NAMESPACE_DISTRHO

SourceList::SourceList(Widget *widget)
    : WAIVEWidget(widget),
      margin(5.0f),
      padding(5.f),
      rowHeight(30.f),
      scrollBarWidth(8),
      scrolling(false),
      callback(nullptr)
{
    loadSharedResources();

    download = new WAIVEImage(this, download_icon, download_icon_len, 512, 344, IMAGE_GENERATE_MIPMAPS);
}

void SourceList::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(background_color);
    rect(0, 0, width, height);
    fill();
    stroke();
    closePath();

    if (source_info == nullptr || source_info_mtx == nullptr)
        return;

    float rowWidth = width - 2.f * padding - scrollBarWidth;
    rowHeight = 2.f * font_size;

    int maxDisplay = height / (rowHeight + margin);

    source_info_mtx->lock();
    int n = source_info->size();

    if (n < maxDisplay)
        scrollPos = 0.f;
    else
        clampScrollPos();

    if (n == 0)
    {
        beginPath();
        fillColor(text_color);
        fontSize(font_size);
        textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_CENTER);
        text(width / 2.f, height / 2.f, "no results found", nullptr);
        closePath();

        source_info_mtx->unlock();

        return;
    }

    float y = 0.f;
    float x = padding;

    int startIndex = scrollPos / (rowHeight + margin);

    for (int i = startIndex; i < startIndex + maxDisplay + 2; i++)
    {
        y = i * (rowHeight + margin) + padding - scrollPos;

        if (y < -rowHeight)
            continue;
        if (y > height || i >= n)
            break;
        drawSourceInfo((source_info->at(i)), x, y, rowWidth, rowHeight, highlighting == i);
    }

    // scroll bar
    if (n > maxDisplay)
    {
        float steps = height / n;

        beginPath();
        fillColor(stroke_color);
        rect(
            width - scrollBarWidth,
            (scrollPos / (rowHeight + margin)) * steps,
            scrollBarWidth,
            steps * maxDisplay);
        fill();
        closePath();
    }

    source_info_mtx->unlock();
}

void SourceList::drawSourceInfo(
    const SourceInfo &info, float x, float y, float width, float height, bool highlight)
{
    translate(x, y);

    beginPath();
    strokeWidth(1.f);
    strokeColor(highlight ? accent_color : stroke_color);
    roundedRect(0, 0, width, height, 4.f);
    stroke();
    closePath();

    beginPath();
    fillColor(text_color);
    moveTo(8, 10);
    lineTo(8, height - 10);
    lineTo(23, height / 2);
    fill();
    closePath();

    std::string infoString = info.archive + ": " + info.name;

    beginPath();
    fillColor(text_color);
    fontSize(font_size);
    textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_LEFT);
    text(30.f, height / 2.0f, infoString.c_str(), nullptr);
    closePath();

    if (info.downloaded == DownloadState::NOT_DOWNLOADED)
    {
        globalTint(Color(0, 0, 0));
        download->align = Align::ALIGN_RIGHT | Align::ALIGN_MIDDLE;
        download->drawAt(width - 5, height / 2.f, 26);
        globalTint(Color(255, 255, 255));
    }
    else if (info.downloaded == DownloadState::DOWNLOADING)
    {
        beginPath();
        strokeColor(stroke_color);
        strokeWidth(3.0f);
        circle(width - 20.f, height / 2.f, 10.f);
        stroke();
        closePath();
    }
    else
    {
        beginPath();
        strokeColor(stroke_color);
        strokeWidth(3.0f);
        moveTo(width - 15.f, 10.f);
        lineTo(width - 10.f, height / 2.f);
        lineTo(width - 15.f, height - 10.f);
        stroke();
        closePath();
    }

    resetTransform();
}

bool SourceList::onScroll(const ScrollEvent &ev)
{
    if (!contains(ev.pos))
        return false;

    scrollPos -= ev.delta.getY() * 10;
    clampScrollPos();

    highlighting = (scrollPos + ev.pos.getY()) / (rowHeight + margin);

    repaint();

    return true;
}

void SourceList::clampScrollPos()
{
    scrollPos = std::min(scrollPos, (source_info->size()) * (rowHeight + margin) + 2 * padding - getHeight());
    scrollPos = std::max(scrollPos, 0.f);
}

bool SourceList::onMotion(const MotionEvent &ev)
{
    if (!isVisible())
        return false;

    if (scrolling)
    {
        scrollPos = (source_info->size() * rowHeight) * ev.pos.getY() / getHeight();
        clampScrollPos();

        repaint();

        return true;
    }
    else if (contains(ev.pos))
    {
        int hl = (scrollPos + ev.pos.getY()) / (rowHeight + margin);
        if (highlighting != hl)
        {
            highlighting = hl;
            repaint();
        }
    }
    else
    {
        if (highlighting != -1)
        {
            highlighting = -1;
            repaint();
        }
    }

    return false;
}

bool SourceList::onMouse(const MouseEvent &ev)
{
    if (!isVisible())
        return false;

    if (!scrolling && ev.press && contains(ev.pos))
    {
        if (ev.pos.getX() > getWidth() - scrollBarWidth)
        {
            scrolling = true;
            return true;
        }

        if (ev.pos.getX() > getWidth() - 26 - scrollBarWidth)
        {
            std::cout << "download " << highlighting << std::endl;
            if (callback != nullptr)
                callback->sourceDownload(highlighting);
            return true;
        }

        if (ev.pos.getX() < 30)
        {
            std::cout << "play " << highlighting << std::endl;
            if (callback != nullptr)
                callback->sourcePreview(highlighting);
            return true;
        }
    }
    else if (scrolling && !ev.press)
        scrolling = false;

    return false;
}

void SourceList::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO