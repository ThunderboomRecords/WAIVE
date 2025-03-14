#include "SourceList.hpp"

START_NAMESPACE_DISTRHO

SourceList::SourceList(Widget *widget)
    : WAIVEWidget(widget),
      margin(5.0f),
      padding(5.f),
      rowHeight(30.f),
      scrolling(false),
      scrollGutter(background_color),
      scrollHandle(foreground_color),
      info("No results found"),
      callback(nullptr),
      highlighting(-1),
      selected(-1),
      previewPlaying(-1),
      source_info(nullptr),
      source_info_mtx(nullptr),
      scrollPos(0.f)

{
    download = new WAIVEImage(this, download_icon, download_icon_len, 131, 119, IMAGE_GENERATE_MIPMAPS);

    scrollBarWidth = 3 * scale_factor;
    columnLabel = 36 * scale_factor;
    columnDownload = 30 * scale_factor + 2.f * (scrollBarWidth + 8);
    columnLicense = columnDownload;

    background_color = WaiveColors::grey2;

    random.seed();
}

void SourceList::setSource(int id, bool send_callback)
{
    std::cout << "SourceList::setSource id " << id << std::endl;
    if (id < 0)
        return;

    bool found = false;
    {
        std::lock_guard<std::mutex> lock(*source_info_mtx);
        for (size_t i = 0; i < source_info->size(); i++)
        {
            if (source_info->at(i)->id == id)
            {
                highlighting = i;
                selected = i;
                found = true;
                break;
            }
        }
    }

    if (!found)
        return;

    scrollPos = static_cast<float>(highlighting - 2) * (rowHeight + margin);
    clampScrollPos();

    std::cout << "SourceList::setSource id " << id << " at " << highlighting << ", scrollPos " << scrollPos << std::endl;

    if (send_callback && callback != nullptr)
        callback->sourceLoad(id);

    repaint();
}

void SourceList::computeColumnWidths()
{
    const float width = getWidth();

    fontFaceId(font);
    fontSize(getFontSize() * 0.8f);
    DGL::Rectangle<float> bounds;
    textBounds(0, 0, "LICENSE", NULL, bounds);

    columnLabel = 36 * scale_factor;
    columnDownload = width - (scrollBarWidth * 2.f) - 8 - 11;
    columnLicense = columnDownload - bounds.getWidth() - 4.f;
}

void SourceList::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(background_color);
    rect(0, 0, width, height);
    fill();
    closePath();

    if (renderDebug)
    {
        beginPath();
        strokeColor(accent_color);
        rect(0, 0, width, height);
        stroke();
        closePath();

        beginPath();
        moveTo(columnLabel, 0);
        lineTo(columnLabel, height);
        moveTo(columnLicense, 0);
        lineTo(columnLicense, height);
        moveTo(columnDownload, 0);
        lineTo(columnDownload, height);
        stroke();
        closePath();
    }

    if (source_info == nullptr || source_info_mtx == nullptr)
        return;

    rowHeight = 2.f * getFontSize();

    int maxDisplay = height / (rowHeight + margin);

    std::lock_guard<std::mutex> lock(*source_info_mtx);
    size_t n = source_info->size();

    if (n < maxDisplay)
        scrollPos = 0.f;
    else
        clampScrollPos();

    if (n == 0)
    {
        beginPath();
        fillColor(text_color);
        fontSize(getFontSize());
        textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_CENTER);
        text(width / 2.f, height / 2.f, info.c_str(), nullptr);
        closePath();

        return;
    }

    float x = padding;

    int startIndex = scrollPos / (rowHeight + margin);

    for (int i = startIndex; i < startIndex + maxDisplay + 2; i++)
    {
        float y = i * (rowHeight + margin) + padding - scrollPos;

        if (y < -rowHeight)
            continue;
        if (y > height || i >= n)
            break;

        try
        {
            drawSourceInfo((source_info->at(i)), x, y, getWidth(), rowHeight, (highlighting == i) || (selected == i), (previewPlaying == i));
        }
        catch (const std::out_of_range &e)
        {
            continue;
        }
    }

    // scroll bar
    if (n > maxDisplay)
    {
        beginPath();
        fillColor(scrollGutter);
        roundedRect(width - scrollBarWidth - 4, 4, scrollBarWidth, height - 8, scrollBarWidth / 2.f);
        fill();
        closePath();

        const float minThumbHeight = scrollBarWidth;
        float steps = (height - 8 - minThumbHeight) / n;
        float thumbHeight = std::max(steps * maxDisplay, minThumbHeight);

        beginPath();
        fillColor(scrollHandle);
        roundedRect(
            width - scrollBarWidth - 4,
            (scrollPos / (rowHeight + margin)) * steps + 4,
            scrollBarWidth,
            thumbHeight,
            scrollBarWidth / 2.f);
        fill();
        closePath();
    }
}

void SourceList::drawSourceInfo(std::shared_ptr<SourceInfo> info, float x, float y, float width, float height, bool highlight, bool playing)
{
    translate(x, y);

    if (info->description.length() == 0)
    {
        std::cout << "zero length description...\n";
        resetTransform();
        return;
    }

    if (highlight)
    {
        beginPath();
        fillColor(accent_color);
        rect(0, 0, width, height);
        fill();
        closePath();
    }

    beginPath();
    fillColor(text_color);
    fontSize(getFontSize());
    textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_LEFT);
    fontFaceId(font);
    text(columnLabel, height / 2.0f, info->description.c_str(), nullptr);
    closePath();

    // fade string
    Paint fade;
    if (highlight)
        fade = linearGradient(columnLicense - 10, 0, columnLicense - 10 - 2 * scrollBarWidth, 0, accent_color, Color(0, 0, 0, 0.f));
    else
        fade = linearGradient(width - scrollBarWidth - 10, 0, width - scrollBarWidth - 10 - 2 * scrollBarWidth, 0, background_color, Color(0, 0, 0, 0.f));
    beginPath();
    fillPaint(fade);
    rect(0, 0, width, height);
    fill();
    closePath();

    if (!playing)
    {
        // Play button
        beginPath();
        if (highlight)
            fillColor(text_color);
        else
            fillColor(highlight_color);
        moveTo(15 * scale_factor, 12 * scale_factor);
        lineTo(15 * scale_factor, height - 12 * scale_factor);
        lineTo(22 * scale_factor, height / 2);
        fill();
        closePath();
    }
    else
    {
        // Stop button
        beginPath();
        if (highlight)
            fillColor(text_color);
        else
            fillColor(highlight_color);
        rect(15 * scale_factor, 12 * scale_factor, 7 * scale_factor, 7 * scale_factor);
        fill();
        closePath();
    }

    if (!highlight)
    {
        resetTransform();
        return;
    }

    if (info->license.length())
    {
        // license info button
        beginPath();
        fillColor(text_color);
        fontSize(getFontSize() * 0.8f);
        textAlign(Align::ALIGN_LEFT | Align::ALIGN_MIDDLE);
        text(columnLicense, height / 2.f, "LICENSE", nullptr);
        closePath();
    }

    if (info->downloaded == DownloadState::NOT_DOWNLOADED)
    {
        download->align = Align::ALIGN_LEFT | Align::ALIGN_MIDDLE;
        download->drawAt(columnDownload, height / 2.f, 9 * scale_factor, 11 * scale_factor);
    }
    else if (info->downloaded == DownloadState::DOWNLOADING)
    {
        beginPath();
        strokeColor(text_color);
        strokeWidth(1.0f);
        circle(columnDownload + 5.f * scale_factor, height / 2.f, 5.f * scale_factor);
        stroke();
        closePath();
    }
    else
    {
        float gap = (height - 11.f * scale_factor) / 2.f + 2.f;
        beginPath();
        strokeColor(text_color);
        strokeWidth(1.0f);
        moveTo(columnDownload + 1 * scale_factor, gap);
        lineTo(columnDownload + 7 * scale_factor, height / 2.f);
        lineTo(columnDownload + 1 * scale_factor, height - gap);
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

        return false;
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

    if (source_info->empty())
        return false;

    if (!scrolling && ev.press && contains(ev.pos))
    {
        if (ev.pos.getX() > getWidth() - scrollBarWidth - 8)
        {
            scrolling = true;
            return false;
        }
        else if (ev.pos.getX() > columnLicense && ev.pos.getX() < columnDownload && source_info->at(highlighting)->license.length())
        {
            try
            {
                // std::cout << "License: " << source_info->at(highlighting)->license << std::endl;
                SystemOpenURL(source_info->at(highlighting)->license);
                return false;
            }
            catch (const std::out_of_range &e)
            {
                std::cerr << e.what() << '\n';
                return false;
            }
        }
        else if (ev.pos.getX() > columnLabel)
        {
            try
            {
                if (source_info->at(highlighting)->downloaded == DownloadState::DOWNLOADED)
                {
                    selected = highlighting;
                    if (callback != nullptr)
                        callback->sourceLoad(source_info->at(highlighting)->id);
                    return true;
                }
                else if (source_info->at(highlighting)->downloaded == DownloadState::NOT_DOWNLOADED)
                {
                    selected = highlighting;
                    if (callback != nullptr)
                        callback->sourceDownload(highlighting);
                    return true;
                }
            }
            catch (const std::out_of_range &e)
            {
                highlighting = -1;
                selected = -1;
                std::cerr << e.what() << '\n';
            }

            return false;
        }
        else if (ev.pos.getX() <= columnLabel)
        {
            if (previewPlaying == highlighting)
            {
                // stop preview
                if (callback != nullptr)
                    callback->sourcePreview(highlighting, false);
                previewPlaying = -1;
            }
            else
            {
                // Play preview
                if (callback != nullptr)
                    callback->sourcePreview(highlighting, true);
                previewPlaying = highlighting;
            }

            repaint();
            return true;
        }
    }
    else if (scrolling && !ev.press)
        scrolling = false;

    return false;
}

void SourceList::selectRandom()
{
    if (source_info->size() == 0)
        return;

    highlighting = random.next() % source_info->size();
    selected = highlighting;
    scrollPos = (highlighting - 2) * (rowHeight + margin) + 2 * padding;
    clampScrollPos();

    repaint();

    if (source_info->at(highlighting)->downloaded == DownloadState::DOWNLOADED)
    {
        if (callback != nullptr)
            callback->sourceLoad(source_info->at(highlighting)->id);
    }
    else if (source_info->at(highlighting)->downloaded == DownloadState::NOT_DOWNLOADED)
    {
        if (callback != nullptr)
            callback->sourceDownload(highlighting);
    }
}

void SourceList::idleCallback() {}

void SourceList::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO