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
      selected(-1)
{
    download = new WAIVEImage(this, download_icon, download_icon_len, 131, 119, IMAGE_GENERATE_MIPMAPS);

    scrollBarWidth = 8 * scale_factor;
    columnLabel = 30 * scale_factor;
    columnLicense = 30 * scale_factor + 2.f * scrollBarWidth;

    random.seed();
}

void SourceList::onNanoDisplay()
{
    const float width = getWidth();
    const float height = getHeight();

    beginPath();
    fillColor(background_color);
    roundedRect(0, 0, width, height, scrollBarWidth / 2.f);
    fill();
    closePath();

    if (renderDebug)
    {
        beginPath();
        strokeColor(accent_color);
        rect(0, 0, width, height);
        stroke();
        closePath();
    }

    if (source_info == nullptr || source_info_mtx == nullptr)
        return;

    float rowWidth = width - 2.f * padding - scrollBarWidth;
    rowHeight = 2.f * getFontSize();

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
        fontSize(getFontSize());
        textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_CENTER);
        text(width / 2.f, height / 2.f, info.c_str(), nullptr);
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

        try
        {
            drawSourceInfo((source_info->at(i)), x, y, rowWidth, rowHeight, (highlighting == i) || (selected == i));
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
        roundedRect(width - scrollBarWidth, 0, scrollBarWidth, height, scrollBarWidth / 2.f);
        fill();
        closePath();

        float steps = height / n;

        beginPath();
        fillColor(scrollHandle);
        roundedRect(
            width - scrollBarWidth,
            (scrollPos / (rowHeight + margin)) * steps,
            scrollBarWidth,
            steps * maxDisplay,
            scrollBarWidth / 2.f);
        fill();
        closePath();
    }

    source_info_mtx->unlock();
}

void SourceList::drawSourceInfo(const SourceInfo &info, float x, float y, float width, float height, bool highlight)
{
    translate(x, y);

    if (info.description.length() == 0)
    {
        std::cout << "zero length description...\n";
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

    // std::string infoString = info.description;

    beginPath();
    fillColor(text_color);
    fontSize(getFontSize());
    textAlign(Align::ALIGN_MIDDLE | Align::ALIGN_LEFT);
    fontFaceId(font);
    text(columnLabel, height / 2.0f, info.description.c_str(), nullptr);
    closePath();

    // fade string
    Paint fade;
    if (highlight)
        fade = linearGradient(width - columnLicense - 50 * scale_factor, 0, width - columnLicense - 2 * scrollBarWidth - 50 * scale_factor, 0, accent_color, Color(0, 0, 0, 0.f));
    else
        fade = linearGradient(width - scrollBarWidth, 0, width - 4.f * scrollBarWidth, 0, background_color, Color(0, 0, 0, 0.f));
    beginPath();
    fillPaint(fade);
    rect(0, 0, width, height);
    fill();
    closePath();

    if (!highlight)
    {
        resetTransform();
        return;
    }

    // play button
    beginPath();
    fillColor(text_color);
    moveTo(8 * scale_factor, 10 * scale_factor);
    lineTo(8 * scale_factor, height - 10 * scale_factor);
    lineTo(20 * scale_factor, height / 2);
    fill();
    closePath();

    if (info.license.length())
    {
        // license info button
        beginPath();
        fillColor(text_color);
        fontSize(getFontSize() * 0.8f);
        textAlign(Align::ALIGN_RIGHT | Align::ALIGN_MIDDLE);
        text(width - columnLicense, height / 2.f, "License", nullptr);
        closePath();
    }

    if (info.downloaded == DownloadState::NOT_DOWNLOADED)
    {
        download->align = Align::ALIGN_RIGHT | Align::ALIGN_MIDDLE;
        download->drawAt(width - scrollBarWidth * 2.f, height / 2.f, getFontSize() * 0.8f);
    }
    else if (info.downloaded == DownloadState::DOWNLOADING)
    {
        beginPath();
        strokeColor(text_color);
        strokeWidth(3.0f);
        circle(width - 8.f - 2.f * scrollBarWidth, height / 2.f, 8.f * scale_factor);
        stroke();
        closePath();
    }
    else
    {
        beginPath();
        strokeColor(text_color);
        strokeWidth(3.0f);
        moveTo(width - scrollBarWidth * 2.f - 12.f * scale_factor, 10.f * scale_factor);
        lineTo(width - scrollBarWidth * 2.f - 4.f * scale_factor, height / 2.f);
        lineTo(width - scrollBarWidth * 2.f - 12.f * scale_factor, height - 10.f * scale_factor);
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

    if (source_info->empty())
        return true;

    if (!scrolling && ev.press && contains(ev.pos))
    {
        if (ev.pos.getX() > getWidth() - scrollBarWidth)
        {
            scrolling = true;
            return true;
        }
        else if (ev.pos.getX() > getWidth() - columnLicense - 50 && source_info->at(highlighting).license.length())
        {
            try
            {
                std::cout << "License: " << source_info->at(highlighting).license << std::endl;
                SystemOpenURL(source_info->at(highlighting).license);
                return true;
            }
            catch (const std::out_of_range &e)
            {
                std::cerr << e.what() << '\n';
                return true;
            }
        }
        else if (ev.pos.getX() > columnLabel)
        {
            try
            {
                if (source_info->at(highlighting).downloaded == DownloadState::DOWNLOADED)
                {
                    selected = highlighting;
                    if (callback != nullptr)
                        callback->sourceLoad(highlighting);
                    return true;
                }
                else if (source_info->at(highlighting).downloaded == DownloadState::NOT_DOWNLOADED)
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
        else
        {
            if (callback != nullptr)
                callback->sourcePreview(highlighting);
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

    if (source_info->at(highlighting).downloaded == DownloadState::DOWNLOADED)
    {
        if (callback != nullptr)
            callback->sourceLoad(highlighting);
    }
    else if (source_info->at(highlighting).downloaded == DownloadState::NOT_DOWNLOADED)
    {
        if (callback != nullptr)
            callback->sourceDownload(highlighting);
    }
}

void SourceList::setCallback(Callback *cb)
{
    callback = cb;
}

END_NAMESPACE_DISTRHO